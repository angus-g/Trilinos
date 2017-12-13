#include "ShyLU_NodeTacho_config.h"

#include <Kokkos_Core.hpp>
#include <impl/Kokkos_Timer.hpp>

#include "TachoExp_Util.hpp"
#include "TachoExp_DenseMatrixView.hpp"
#include "TachoExp_DenseFlopCount.hpp"

#include "TachoExp_Chol_ByBlocks.hpp"
#include "TachoExp_Gemm_ByBlocks.hpp"
#include "TachoExp_Herk_ByBlocks.hpp"
#include "TachoExp_Trsm_ByBlocks.hpp"

#include "TachoExp_CommandLineParser.hpp" 

#ifdef TACHO_HAVE_MKL
#include "mkl_service.h"
#endif

using namespace Tacho;
using namespace Tacho::Experimental;

#define RESET_TIME_PER_THREAD                   \
  if (iter <= 0)                                \
    resetTimePerThread();

#define PRINT_TIME_PER_THREAD                   \
  double t_total = 0, t_avg, t_min = 0, t_max = 0;                      \
  getTimePerThread(nthreads, t_total, t_avg, t_min, t_max);             \
  t_total /= niter; t_avg /= niter; t_min /= niter; t_max /= niter;     \
                                                                        \
  printf("  Time per thread\n");                                        \
  printf("       min, max, avg, byblocks                         %10.6f s, %10.6f s, %10.6f s, %10.6f s\n", t_min, t_max, t_avg, t_byblocks); \
  printf("       sum(time per thread)/(byblocks x nthreads):     %10.6f\n", t_total/(t_byblocks*nthreads)); \
  printf("       byblocks/reference (speedup):                   %10.6f\n", t_reference/t_byblocks); \
  printf("\n");                                                         

  // for (ordinal_type i=0;i<nthreads;++i)                                 
  //    printf("  time for external blas lapack (diff from avg):       %10.6f s, %10.6f s\n", g_time_per_thread[i]/niter, fabs(g_time_per_thread[i] - t_avg)/niter); 


int main (int argc, char *argv[]) {
  CommandLineParser opts("This example program measure the performance of dense-by-blocks on Kokkos::OpenMP");  

  bool serial = false;
  int nthreads = 1;
  bool verbose = true;
  int mbeg = 1000;
  int mend = 6000;
  int step = 1000;
  int mb = 128;

  opts.set_option<bool>("serial", "Flag for invoking serial algorithm", &serial);
  opts.set_option<int>("kokkos-threads", "Number of threads", &nthreads);
  opts.set_option<bool>("verbose", "Flag for verbose printing", &verbose);
  opts.set_option<int>("begin", "Test problem begin size", &mbeg);  
  opts.set_option<int>("end", "Test problem end size", &mend);  
  opts.set_option<int>("step", "Test problem step size", &step);  
  opts.set_option<int>("mb", "Blocksize", &mb);  

  const bool r_parse = opts.parse(argc, argv);
  if (r_parse) return 0; // print help return 

  Kokkos::initialize(argc, argv);
  if (std::is_same<Kokkos::DefaultHostExecutionSpace,Kokkos::Serial>::value) 
    std::cout << "Kokkos::Serial\n";
  else
    Kokkos::DefaultHostExecutionSpace::print_configuration(std::cout, false);
  
  int r_val = 0;
  
  {
    typedef double value_type;
    typedef Kokkos::DefaultHostExecutionSpace exec_space;
    typedef DenseMatrixView<value_type,exec_space>          DenseMatrixViewType;
    typedef DenseMatrixView<DenseMatrixViewType,exec_space> DenseMatrixOfBlocksType;
    typedef Kokkos::pair<ordinal_type,ordinal_type> range_type;

    Kokkos::Impl::Timer timer;

    typedef Kokkos::TaskScheduler<exec_space> sched_type;
    sched_type sched;

    typedef TaskFunctor_Chol<sched_type,DenseMatrixOfBlocksType,
      Uplo::Upper,Algo::ByBlocks> task_functor_chol;
    typedef TaskFunctor_Trsm<sched_type,double,DenseMatrixOfBlocksType,
      Side::Left,Uplo::Upper,Trans::ConjTranspose,Diag::NonUnit,Algo::ByBlocks> task_functor_trsm;
    typedef TaskFunctor_Gemm<sched_type,double,DenseMatrixOfBlocksType,
      Trans::NoTranspose,Trans::NoTranspose,Algo::ByBlocks> task_functor_gemm;

    const ordinal_type max_functor_size = 4*sizeof(task_functor_gemm);
    
    Kokkos::View<value_type*,exec_space> 
      a("a", mend*mend), a1("a1", mend*mend), a2("a2", mend*mend), 
      b("b", mend*mend);
    
    const ordinal_type bmend = (mend/mb) + 1;
    Kokkos::View<DenseMatrixViewType*,exec_space> 
      ha("ha", bmend*bmend), hb("hb", bmend*bmend), hc("hc", bmend*bmend);

    {
      const ordinal_type
        task_queue_capacity = bmend*bmend*bmend*max_functor_size,
        min_block_size  = 16,
        max_block_size  = 4*max_functor_size,
        num_superblock  = 4,
        superblock_size = task_queue_capacity/num_superblock;
      
      sched = sched_type(typename exec_space::memory_space(),
                         task_queue_capacity,
                         min_block_size,
                         max_block_size,
                         superblock_size);
    }

    const ordinal_type dry = -2, niter = 3;

    double t_reference = 0, t_byblocks = 0;

    Random<value_type> random;
    auto randomize = [&](const DenseMatrixViewType &mat) {
      const ordinal_type m = mat.dimension_0(), n = mat.dimension_1();
      for (ordinal_type j=0;j<n;++j)
        for (ordinal_type i=0;i<m;++i)
          mat(i,j) = random.value();
    };

    ///
    /// Chol
    ///
    for (ordinal_type m=mbeg;m<=mend;m+=step) {
      t_reference = 0; t_byblocks = 0;
      auto sub_a  = Kokkos::subview(a,  range_type(0,m*m));
      auto sub_a1 = Kokkos::subview(a1, range_type(0,m*m));
      auto sub_a2 = Kokkos::subview(a2, range_type(0,m*m));
      
      {
        Kokkos::deep_copy(sub_a, 0);

        DenseMatrixViewType A;
        A.set_view(m, m);
        A.attach_buffer(1, m, a.data());
        for (ordinal_type i=0;i<m;++i) {
          A(i,i) = 4;
          const ordinal_type ip = i+1;
          if (ip < m) {
            A(ip,i ) = 1;
            A(i ,ip) = 1;
          }
        }
      }

      // reference 
      {
        int dummy = 0;
        
        DenseMatrixViewType A;
        A.set_view(m, m);
        A.attach_buffer(1, m, a1.data());

        for (ordinal_type iter=dry;iter<niter;++iter) {
          Kokkos::deep_copy(sub_a1, sub_a);
          timer.reset();
          Chol<Uplo::Upper,Algo::External>::invoke(dummy, dummy, A);
          t_reference += (iter >= 0)*timer.seconds();
        }
        t_reference /= niter;
      }
      
      // dense by blocks
      {
        DenseMatrixViewType A;
        A.set_view(m, m);
        A.attach_buffer(1, m, a2.data());

        DenseMatrixOfBlocksType HA;
        const ordinal_type bm = (m/mb) + (m%mb>0);
        HA.set_view(bm, bm);
        HA.attach_buffer(1, bm, ha.data());
        {          
          for (ordinal_type iter=dry;iter<niter;++iter) {
            Kokkos::deep_copy(sub_a2, sub_a);

#if defined( TACHO_PROFILE_TIME_PER_THREAD )
            RESET_TIME_PER_THREAD;
#endif
            timer.reset();
            setMatrixOfBlocks(HA, m, m, mb);
            attachBaseBuffer(HA, A.data(), A.stride_0(), A.stride_1());

            Kokkos::host_spawn(Kokkos::TaskTeam(sched, Kokkos::TaskPriority::High),
                               task_functor_chol(sched, HA));
            Kokkos::wait(sched);
            t_byblocks += (iter >=0)*timer.seconds();
            
            clearFutureOfBlocks(HA);
          }
          t_byblocks /= niter;
        }
      }

      {
        double diff = 0.0, norm = 0.0;
        for (ordinal_type p=0;p<(m*m);++p) {
          norm += a1(p)*a1(p);
          diff += (a1(p) - a2(p))*(a1(p) - a2(p));
        }
        const double relerr = sqrt(diff/norm), eps = std::numeric_limits<double>::epsilon()*1000;

        if (sqrt(diff/norm) > eps) {
          printf("******* chol problem %d fails, reltaive error against reference is %10.4f\n", 
                 m, relerr);
          r_val = -1;
          break;
        }
      }
      
      {
        const double kilo = 1024, gflop = DenseFlopCount<value_type>::Chol(m)/kilo/kilo/kilo;
        printf("chol problem %10d, gflop %10.2f, gflop/s :: reference %10.2f, byblocks %10.2f\n", 
               m, gflop, gflop/t_reference, gflop/t_byblocks);
#if defined( TACHO_PROFILE_TIME_PER_THREAD )
        PRINT_TIME_PER_THREAD;
#endif
      }
    }
    printf("\n\n");

    ///
    /// Trsm
    ///
    
    for (ordinal_type m=mbeg;m<=mend;m+=step) {
      t_reference = 0; t_byblocks = 0;
      auto sub_a  = Kokkos::subview(a,  range_type(0,m*m));
      auto sub_a1 = Kokkos::subview(a1, range_type(0,m*m));
      auto sub_a2 = Kokkos::subview(a2, range_type(0,m*m));
      
      {
        DenseMatrixViewType A, B;
        A.set_view(m, m);
        A.attach_buffer(1, m, a.data());

        B.set_view(m, m);
        B.attach_buffer(1, m, a1.data());

        randomize(A);
        randomize(B);

        Kokkos::deep_copy(a2, a1);
      }

      // reference 
      {
        int dummy = 0;
        
        DenseMatrixViewType A, B;
        A.set_view(m, m);
        A.attach_buffer(1, m, a.data());

        B.set_view(m, m);
        B.attach_buffer(1, m, a1.data());
        
        const double alpha = -1.0;

        for (ordinal_type iter=dry;iter<niter;++iter) {
          timer.reset();
          Trsm<Side::Left,Uplo::Upper,Trans::ConjTranspose,Algo::External>
            ::invoke(dummy, dummy, Diag::NonUnit(), alpha, A, B);
          t_reference += (iter >= 0)*timer.seconds();
        }
        t_reference /= niter;
      }
      
      // dense by blocks
      {
        DenseMatrixViewType A, B;
        A.set_view(m, m);
        A.attach_buffer(1, m, a.data());

        B.set_view(m, m);
        B.attach_buffer(1, m, a2.data());

        DenseMatrixOfBlocksType HA, HB;
        const ordinal_type bm = (m/mb) + (m%mb>0);

        HA.set_view(bm, bm);
        HA.attach_buffer(1, bm, ha.data());

        HB.set_view(bm, bm);
        HB.attach_buffer(1, bm, hb.data());
        {
          const double alpha = -1.0;

          for (ordinal_type iter=dry;iter<niter;++iter) {
#if defined( TACHO_PROFILE_TIME_PER_THREAD )
            RESET_TIME_PER_THREAD;
#endif
            timer.reset();
            setMatrixOfBlocks(HA, m, m, mb);
            attachBaseBuffer(HA, A.data(), A.stride_0(), A.stride_1());
            
            setMatrixOfBlocks(HB, m, m, mb);
            attachBaseBuffer(HB, B.data(), B.stride_0(), B.stride_1());

            Kokkos::host_spawn(Kokkos::TaskTeam(sched, Kokkos::TaskPriority::High),
                               task_functor_trsm(sched, alpha, HA, HB));
            Kokkos::wait(sched);
            t_byblocks += (iter >=0)*timer.seconds();

            clearFutureOfBlocks(HB);
          }
          t_byblocks /= niter;
        }
      }
      
      {
        double diff = 0.0, norm = 0.0;
        for (ordinal_type p=0;p<(m*m);++p) {
          norm += a1(p)*a1(p);
          diff += (a1(p) - a2(p))*(a1(p) - a2(p));
        }
        const double relerr = sqrt(diff/norm), eps = std::numeric_limits<double>::epsilon()*1000;

        if (sqrt(diff/norm) > eps) 
          printf("******* trsm problem %d fails, reltaive error against reference is %10.4f\n", 
                 m, relerr);
      }
      
      {
        const double kilo = 1024, gflop = DenseFlopCount<value_type>::Trsm(true, m, m)/kilo/kilo/kilo;
        printf("trsm problem %10d, gflop %10.2f, gflop/s :: reference %10.2f, byblocks %10.2f\n", 
               m, gflop, gflop/t_reference, gflop/t_byblocks);
#if defined( TACHO_PROFILE_TIME_PER_THREAD )
        PRINT_TIME_PER_THREAD;
#endif
      }
    }
    printf("\n\n");

    ///
    /// Gemm
    ///
    
    for (ordinal_type m=mbeg;m<=mend;m+=step) {
      t_reference = 0; t_byblocks = 0;
      auto sub_a  = Kokkos::subview(a,  range_type(0,m*m));
      auto sub_b  = Kokkos::subview(b,  range_type(0,m*m));
      auto sub_a1 = Kokkos::subview(a1, range_type(0,m*m));
      auto sub_a2 = Kokkos::subview(a2, range_type(0,m*m));

      {
        DenseMatrixViewType A, B, C;
        A.set_view(m, m);
        A.attach_buffer(1, m, a.data());

        B.set_view(m, m);
        B.attach_buffer(1, m, b.data());

        C.set_view(m, m);
        C.attach_buffer(1, m, a1.data());

        randomize(A);
        randomize(B);
        randomize(C);

        Kokkos::deep_copy(a2, a1);
      }

      // reference 
      {
        int dummy = 0;
        
        DenseMatrixViewType A, B, C;
        A.set_view(m, m);
        A.attach_buffer(1, m, a.data());

        B.set_view(m, m);
        B.attach_buffer(1, m, b.data());

        C.set_view(m, m);
        C.attach_buffer(1, m, a1.data());
        
        const double alpha = -1.0, beta = 1.0;

        for (ordinal_type iter=dry;iter<niter;++iter) {
          timer.reset();
          Gemm<Trans::NoTranspose,Trans::NoTranspose,Algo::External>
            ::invoke(dummy, dummy, alpha, A, B, beta, C);
          t_reference += (iter >= 0)*timer.seconds();
        }
        t_reference /= niter;
      }
      
      // dense by blocks
      {
        DenseMatrixViewType A, B, C;
        A.set_view(m, m);
        A.attach_buffer(1, m, a.data());

        B.set_view(m, m);
        B.attach_buffer(1, m, b.data());

        C.set_view(m, m);
        C.attach_buffer(1, m, a2.data());

        DenseMatrixOfBlocksType HA, HB, HC;
        const ordinal_type bm = (m/mb) + (m%mb>0);

        HA.set_view(bm, bm);
        HA.attach_buffer(1, bm, ha.data());

        HB.set_view(bm, bm);
        HB.attach_buffer(1, bm, hb.data());

        HC.set_view(bm, bm);
        HC.attach_buffer(1, bm, hc.data());

        // HA.set_view(bm, 1);
        // HA.attach_buffer(1, bm, ha.data());

        // HB.set_view(1, bm);
        // HB.attach_buffer(1, bm, hb.data());

        // HC.set_view(bm, bm);
        // HC.attach_buffer(1, bm, hc.data());
        {
          const double alpha = -1.0, beta = 1.0;

          for (ordinal_type iter=dry;iter<niter;++iter) {
#if defined( TACHO_PROFILE_TIME_PER_THREAD )
            RESET_TIME_PER_THREAD;
#endif
            timer.reset();

            setMatrixOfBlocks(HA, m, m, mb);
            //setMatrixOfBlocks(HA, m, m, mb, m);
            attachBaseBuffer(HA, A.data(), A.stride_0(), A.stride_1());
            
            setMatrixOfBlocks(HB, m, m, mb);
            //setMatrixOfBlocks(HB, m, m, m, mb);
            attachBaseBuffer(HB, B.data(), B.stride_0(), B.stride_1());
            
            setMatrixOfBlocks(HC, m, m, mb);
            //setMatrixOfBlocks(HC, m, m, mb, mb);
            attachBaseBuffer(HC, C.data(), C.stride_0(), C.stride_1());

            Kokkos::host_spawn(Kokkos::TaskTeam(sched, Kokkos::TaskPriority::High),
                               task_functor_gemm(sched, alpha, HA, HB, beta, HC));
            Kokkos::wait(sched);
            t_byblocks += (iter >=0)*timer.seconds();

            clearFutureOfBlocks(HC);
          }
          t_byblocks /= niter;
        }
      }
      
      {
        double diff = 0.0, norm = 0.0;
        for (ordinal_type p=0;p<(m*m);++p) {
          norm += a1(p)*a1(p);
          diff += (a1(p) - a2(p))*(a1(p) - a2(p));
        }
        const double relerr = sqrt(diff/norm), eps = std::numeric_limits<double>::epsilon()*1000;

        if (sqrt(diff/norm) > eps) 
          printf("******* gemm problem %d fails, reltaive error against reference is %10.4f\n", 
                 m, relerr);
      }
      
      {
        const double kilo = 1024, gflop = DenseFlopCount<value_type>::Gemm(m, m, m)/kilo/kilo/kilo;
        printf("gemm problem %10d, gflop %10.2f, gflop/s :: reference %10.2f, byblocks %10.2f\n", 
               m, gflop, gflop/t_reference, gflop/t_byblocks);
#if defined( TACHO_PROFILE_TIME_PER_THREAD )
        PRINT_TIME_PER_THREAD;
#endif
      }
    }
    printf("\n\n");
  }
  Kokkos::finalize();

  return r_val;
}