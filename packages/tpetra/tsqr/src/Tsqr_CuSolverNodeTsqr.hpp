//@HEADER
// ************************************************************************
//
//          Kokkos: Node API and Parallel Node Kernels
//              Copyright (2008) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ************************************************************************
//@HEADER

/// \file Tsqr_CuSolverNodeTsqr.hpp
/// \brief Declaration and definition of CuSolverNodeTsqr.

#ifndef TSQR_CUSOLVERNODETSQR_HPP
#define TSQR_CUSOLVERNODETSQR_HPP

#include "TpetraTSQR_config.h"

#if defined(HAVE_TPETRATSQR_CUBLAS) && defined(HAVE_TPETRATSQR_CUSOLVER)
#include "Tsqr_NodeTsqr.hpp"
#include "Tsqr_Impl_CuBlas.hpp"
#include "Tsqr_Impl_CuSolver.hpp"
#include "Kokkos_ArithTraits.hpp"
#include <memory>
#include <type_traits>

namespace TSQR {
  namespace Impl {
    using cusolver_memory_space = Kokkos::CudaSpace;
    using cusolver_execution_space = Kokkos::Cuda;

    template<class Scalar>
    using non_const_kokkos_value_type = typename Kokkos::ArithTraits<
        typename std::remove_const<Scalar>::type
      >::val_type;

    template<class Scalar>
    using kokkos_view_value_type = typename std::conditional<
        std::is_const<Scalar>::value,
        const non_const_kokkos_value_type<Scalar>,
        non_const_kokkos_value_type<Scalar>
      >::type;

    template<class T, class MemorySpace>
    using matrix_type = Kokkos::View<T**, Kokkos::LayoutLeft, MemorySpace>;

    template<class T>
    using device_matrix_type = matrix_type<T, cusolver_memory_space>;

    template<class T>
    void
    reallocDeviceMatrixIfNeeded (device_matrix_type<T>& mat,
                                 const char label[],
                                 const size_t minNumRows,
                                 const size_t minNumCols)
    {
      using Kokkos::view_alloc;
      using Kokkos::WithoutInitializing;

      if (size_t (mat.extent (0)) < minNumRows ||
          size_t (mat.extent (1)) < minNumCols) {
        mat = device_matrix_type<T> ();
        auto alloc =
          view_alloc (std::string (label), WithoutInitializing);
        mat = device_matrix_type<T> (alloc, minNumRows, minNumCols);
      }
    }

    template<class T, class MemorySpace>
    using mat_view_type =
      Kokkos::View<T**, Kokkos::LayoutLeft, MemorySpace,
                   Kokkos::MemoryTraits<Kokkos::Unmanaged>>;

    template<class T>
    using device_mat_view_type = mat_view_type<T, cusolver_memory_space>;

    template<class T>
    using host_mat_view_type = mat_view_type<T, Kokkos::HostSpace>;

    template<class Scalar, class MemorySpace>
    static mat_view_type<kokkos_view_value_type<Scalar>, MemorySpace>
    get_mat_view (const size_t nrows,
                  const size_t ncols,
                  Scalar A[],
                  const size_t lda)
    {
      static_assert
        (! std::is_const<non_const_kokkos_value_type<Scalar> >::value,
         "non_const_kokkos_value_type is const.");
      using KVVT = kokkos_view_value_type<Scalar>; // preserves const
      static_assert
        ((std::is_const<Scalar>::value && std::is_const<KVVT>::value) ||
         (! std::is_const<Scalar>::value && ! std::is_const<KVVT>::value),
         "kokkos_view_value_type failed to preserve const-ness.");
      KVVT* A_raw = reinterpret_cast<KVVT*> (A);

      mat_view_type<KVVT, MemorySpace> A_full (A_raw, lda, ncols);
      const std::pair<size_t, size_t> rowRange (0, nrows);
      return Kokkos::subview (A_full, rowRange, Kokkos::ALL ());
    }

    template<class Scalar>
    static host_mat_view_type<kokkos_view_value_type<Scalar>>
    get_host_mat_view (const size_t nrows,
                       const size_t ncols,
                       Scalar A[],
                       const size_t lda)
    {
      return get_mat_view<Scalar, Kokkos::HostSpace> (nrows, ncols, A, lda);
    }

    template<class Scalar>
    static device_mat_view_type<kokkos_view_value_type<Scalar>>
    get_device_mat_view (const size_t nrows,
                         const size_t ncols,
                         Scalar A[],
                         const size_t lda)
    {
      return get_mat_view<Scalar, cusolver_memory_space> (nrows, ncols, A, lda);
    }

    template<class T, class MemorySpace>
    using vector_type = Kokkos::View<T*, MemorySpace>;

    template<class T>
    using device_vector_type = vector_type<T, cusolver_memory_space>;

    template<class T>
    void
    reallocDeviceVectorIfNeeded (device_vector_type<T>& vec,
                                 const char label[],
                                 const size_t minSize)
    {
      using Kokkos::view_alloc;
      using Kokkos::WithoutInitializing;

      if (size_t (vec.size ()) < minSize) {
        vec = device_vector_type<T> ();
        auto alloc = view_alloc (std::string (label), WithoutInitializing);
        vec = device_vector_type<T> (alloc, minSize);
      }
    }

    template<class T, class MemorySpace>
    using vec_view_type =
      Kokkos::View<T*, MemorySpace,
                   Kokkos::MemoryTraits<Kokkos::Unmanaged>>;

    template<class T>
    using device_vec_view_type = vec_view_type<T, cusolver_memory_space>;

    using info_type = Kokkos::View<int, cusolver_memory_space>;
    using const_info_type = Kokkos::View<const int, cusolver_memory_space>;

    template<class LocalOrdinal, class Scalar>
    class CuSolverNodeFactorOutput :
      public NodeFactorOutput<LocalOrdinal, Scalar>
    {
    public:
      //using cuda_value_type = typename Impl::CudaValue<Scalar>::type;
      using kokkos_value_type = non_const_kokkos_value_type<Scalar>;
      using const_tau_type = device_vector_type<const kokkos_value_type>;
      using const_unmanaged_tau_type =
        device_vec_view_type<const kokkos_value_type>;

      CuSolverNodeFactorOutput (const const_tau_type& tau,
                                const const_info_type& info) :
        tau_ (tau), info_ (info)
      {}

      const_unmanaged_tau_type tau () const { return tau_; }

      int info () const {
        int info_h = 0;
        Kokkos::deep_copy (info_h, info_);
        return info_h;
      }

    private:
      const_tau_type tau_;
      const_info_type info_;
    };

    template<class ScalarType, class IndexType>
    class SetDiagonalEntriesToOne {
      static_assert (! std::is_const<ScalarType>::value,
        "SetDiagonalEntriesToOne requires a View of nonconst.");
    public:
      SetDiagonalEntriesToOne
        (const device_mat_view_type<ScalarType>& A) : A_ (A) {}
      KOKKOS_INLINE_FUNCTION void
      operator() (const IndexType j) const {
        A_(j,j) = ScalarType (1.0);
      }
    private:
      device_mat_view_type<ScalarType> A_;
    };

    template<class ScalarType>
    void
    set_diagonal_entries_to_one
      (const device_mat_view_type<ScalarType>& A)
    {
      static_assert (! std::is_const<ScalarType>::value,
        "set_diagonal_entries_to_one requires a View of nonconst.");
      using LO =
        typename std::make_signed<decltype (A.extent (1)) >::type;
      const LO ncols = std::min (A.extent (0), A.extent (1));
      using Kokkos::RangePolicy;
      RangePolicy<cusolver_execution_space, LO> range (0, ncols);
      Kokkos::parallel_for
        ("set_diagonal_entries_to_one", range,
         SetDiagonalEntriesToOne<ScalarType, LO> (A));
    }

  } // namespace Impl

  /// \class CuSolverNodeTsqr
  /// \brief NodeTsqr implementation based on cuSOLVER.
  /// \author Mark Hoemmen
  template<class LocalOrdinal, class Scalar>
  class CuSolverNodeTsqr : public NodeTsqr<LocalOrdinal, Scalar>
  {
  private:
    using base_type = NodeTsqr<LocalOrdinal, Scalar>;
    using my_factor_output_type =
      Impl::CuSolverNodeFactorOutput<LocalOrdinal, Scalar>;
    using kokkos_value_type =
      Impl::non_const_kokkos_value_type<Scalar>;

  public:
    using ordinal_type = typename base_type::ordinal_type;
    using scalar_type = typename base_type::scalar_type;
    using factor_output_type = typename base_type::factor_output_type;

    CuSolverNodeTsqr () = default;

    Teuchos::RCP<const Teuchos::ParameterList>
    getValidParameters () const override
    {
      return Teuchos::parameterList ("NodeTsqr");
    }

    void
    setParameterList
      (const Teuchos::RCP<Teuchos::ParameterList>&) override
    {}

    std::string description () const override {
      return "CuSolverNodeTsqr";
    }

    bool wants_device_memory () const override { return true; }

    bool ready () const override {
      return true;
    }

    bool
    QR_produces_R_factor_with_nonnegative_diagonal () const override
    {
      return false;
    }

    size_t cache_size_hint () const override {
      return 0;
    }

  private:
    using tau_type = Impl::device_vector_type<kokkos_value_type>;

    // must return owning, since we'll pass off to factor output
    tau_type
    get_tau (const LocalOrdinal numCols) const
    {
      using Impl::reallocDeviceVectorIfNeeded;
      Impl::reallocDeviceVectorIfNeeded (tau_, "tau", size_t (numCols));
      return tau_;
    }

    using work_type = Impl::device_vector_type<kokkos_value_type>;
    using nonowning_work_type =
      Impl::device_vec_view_type<kokkos_value_type>;

    nonowning_work_type
    get_work_for_geqrf (const LocalOrdinal numRows,
                        const LocalOrdinal numCols,
                        Scalar A[],
                        const LocalOrdinal lda) const
    {
      using TSQR::Impl::CuSolver;
      using TSQR::Impl::CuSolverHandle;

      auto info = get_info ();
      CuSolver<Scalar> solver
        {CuSolverHandle::getSingleton (), info.data ()};
      const int lwork =
        solver.compute_QR_lwork (numRows, numCols, A, lda);
      // Avoid constant reallocation by setting a minimum lwork.
      constexpr int min_lwork = 128;
      const int new_lwork = lwork < min_lwork ? min_lwork : lwork;
      using Impl::reallocDeviceVectorIfNeeded;
      reallocDeviceVectorIfNeeded (work_, "work", new_lwork);
      return nonowning_work_type (work_);
    }

    nonowning_work_type
    get_work_for_apply_Q_factor (const ApplyType& apply_type,
                                 const LocalOrdinal nrows,
                                 const LocalOrdinal ncols_C,
                                 const LocalOrdinal ncols_Q,
                                 const Scalar A[],
                                 const LocalOrdinal lda,
                                 const Scalar tau[],
                                 Scalar C[],
                                 const LocalOrdinal ldc) const
    {
      using TSQR::Impl::CuSolver;
      using TSQR::Impl::CuSolverHandle;

      auto info = get_info ();
      CuSolver<Scalar> solver
        {CuSolverHandle::getSingleton (), info.data ()};
      const char side = 'L';
      const char trans = apply_type.toString ()[0];
      const int lwork =
        solver.apply_Q_factor_lwork (side, trans,
                                     nrows, ncols_C, ncols_Q,
                                     A, lda, tau, C, ldc);
      // Avoid constant reallocation by setting a minimum lwork.
      constexpr int min_lwork = 128;
      const int new_lwork = lwork < min_lwork ? min_lwork : lwork;
      using Impl::reallocDeviceVectorIfNeeded;
      reallocDeviceVectorIfNeeded (work_, "work", new_lwork);
      return nonowning_work_type (work_);
    }

    // must return owning, since we'll pass off to factor output
    Impl::info_type
    get_info () const
    {
      if (info_.data () == nullptr) {
        info_ = Impl::info_type ("info");
      }
      // "get last error" model will avoid doing multiple info allocations.
      return info_;
    }

    Impl::device_mat_view_type<kokkos_value_type>
    get_Q_copy (const LocalOrdinal nrows,
                const LocalOrdinal ncols,
                const Scalar Q[],
                const LocalOrdinal ldq) const
    {
      using Impl::reallocDeviceMatrixIfNeeded;
      reallocDeviceMatrixIfNeeded (Q_copy_, "Q_copy", nrows, ncols);
      auto Q_view = Impl::get_device_mat_view (nrows, ncols, Q, ldq);
      Kokkos::deep_copy (Q_copy_, Q_view);
      return Impl::device_mat_view_type<kokkos_value_type> (Q_copy_);
    }

    Impl::device_mat_view_type<kokkos_value_type>
    get_B_copy (const LocalOrdinal nrows_and_ncols,
                const Scalar B[], // HOST MEMORY
                const LocalOrdinal ldb) const
    {
      using Impl::reallocDeviceMatrixIfNeeded;
      reallocDeviceMatrixIfNeeded (B_copy_, "B_copy",
                                   nrows_and_ncols,
                                   nrows_and_ncols);
      using Impl::get_host_mat_view;
      auto B_view = get_host_mat_view (nrows_and_ncols,
                                       nrows_and_ncols, B, ldb);
      Kokkos::deep_copy (B_copy_, B_view);
      return Impl::device_mat_view_type<kokkos_value_type> (B_copy_);
    }

    void
    extract_R (const LocalOrdinal nrows,
               const LocalOrdinal ncols,
               const Scalar A[], // DEVICE POINTER
               const LocalOrdinal lda,
               Scalar R[], // HOST POINTER
               const LocalOrdinal ldr,
               const bool /* contiguous_cache_blocks */) const
    {
      auto A_view = Impl::get_device_mat_view<const Scalar>
        (nrows, ncols, A, lda);
      auto R_view = Impl::get_host_mat_view<Scalar>
        (ncols, ncols, R, ldr);

      try {
        // Fill R (including lower triangle) with zeros.
        Kokkos::deep_copy (R_view, kokkos_value_type {});
      }
      catch (std::exception& e) {
        std::ostringstream err;
        err << "TSQR::CuSolverNodeTsqr::extract_R: "
          "Kokkos::deep_copy(R_view, 0) threw an exception: "
          << std::endl << e.what ();
        throw std::runtime_error (err.str ());
      }

      // Copy out the upper triangle of the R factor from A into R.
      //
      // The following (pseudo)code does not work:
      //
      // auto A_view_top = subview(A_view, {0, ncols}, ALL());
      // Kokkos::deep_copy(R_view, A_view_top);
      //
      // Kokkos throws an exception, claiming "no available copy
      // mechanism."  This is probably because A_view is not packed.
      // This means that cudaMemcpy won't work, so Kokkos must execute
      // a kernel to copy the data.  However, that kernel must be able
      // to access both Views.  In this case, it (thinks it) can't,
      // because R_view is a HostSpace View and A_view_top is a device
      // View (even though it may be a CudaUVMSpace View).

      using Kokkos::ALL;
      using Kokkos::subview;
      using LO = LocalOrdinal;
      const std::pair<LO, LO> rowRange (0, ncols);
      auto A_view_top = subview (A_view, rowRange, ALL ());
      try {
        Kokkos::deep_copy (R_view, A_view_top);
      }
      catch (std::exception& e) {
        // Packed device version of R.
        using Impl::reallocDeviceMatrixIfNeeded;
        reallocDeviceMatrixIfNeeded (R_copy_, "R_copy", ncols, ncols);
        Kokkos::deep_copy (R_copy_, A_view_top);
        Kokkos::deep_copy (R_view, R_copy_);
      }

      for (LO j = 0; j < ncols; ++j) {
        auto R_j = subview (R_view, Kokkos::ALL (), j);
        for (LO i = j + LO(1); i < LO (R_j.extent(0)); ++i) {
          R_j(i) = kokkos_value_type {};
        }
      }
    }

  public:
    Teuchos::RCP<factor_output_type>
    factor (const LocalOrdinal nrows,
            const LocalOrdinal ncols,
            Scalar A[],
            const LocalOrdinal lda,
            Scalar R[],
            const LocalOrdinal ldr,
            const bool contigCacheBlocks) const override
    {
      // It's a common case to call factor() again and again with the
      // same pointers.  In that case, it's wasteful for us to
      // allocate a new tau array each time, especially since most
      // users want explicit Q anyway (and thus will never see tau).
      auto tau = get_tau (ncols);
      // FIXME (mfh 11 Dec 2019) TSQR::Impl::CuBlas takes
      // std::complex, but Kokkos::View stores Kokkos::complex.  We're
      // assuming they have the same alignment here, but all of Tpetra
      // assumes that.
      Scalar* tau_raw = reinterpret_cast<Scalar*> (tau.data ());
      auto work = get_work_for_geqrf (nrows, ncols, A, lda);
      Scalar* work_raw = reinterpret_cast<Scalar*> (work.data ());
      const int lwork (work.extent (0));
      auto info = get_info ();

      using TSQR::Impl::CuSolver;
      using TSQR::Impl::CuSolverHandle;
      CuSolver<Scalar> solver
        {CuSolverHandle::getSingleton (), info.data ()};
      try {
        solver.compute_QR (nrows, ncols, A, lda, tau_raw,
                           work_raw, lwork);
      }
      catch (std::exception& e) {
        std::ostringstream err;
        err << "TSQR::CuSolverNodeTsqr::factor: CuSolver::compute_QR "
          "threw an exception: " << std::endl << e.what ();
        throw std::runtime_error (err.str ());
      }
      try {
        this->extract_R (nrows, ncols, A, lda, R, ldr,
                         contigCacheBlocks);
      }
      catch (std::exception& e) {
        std::ostringstream err;
        err << "TSQR::CuSolverNodeTsqr::factor: extract_R "
          "threw an exception: " << std::endl << e.what ();
        throw std::runtime_error (err.str ());
      }
      return Teuchos::rcp (new my_factor_output_type (tau, info));
    }

  private:
    const my_factor_output_type&
    get_my_factor_output (const factor_output_type& factor_output) const
    {
      const char prefix[] = "TSQR::CuSolverNodeTsqr: ";

      const my_factor_output_type* output_ptr =
        dynamic_cast<const my_factor_output_type*> (&factor_output);
      if (output_ptr == nullptr) {
        const std::string this_name = Teuchos::typeName (*this);
        const std::string factor_output_type_name =
          Teuchos::TypeNameTraits<my_factor_output_type>::name ();
        const std::string dynamic_type_name =
          Teuchos::demangleName (typeid (factor_output).name ());
        TEUCHOS_TEST_FOR_EXCEPTION
          (true, std::invalid_argument, prefix << "Input "
           "factor_output_type object was not created by the same "
           "type of CuSolverNodeTsqr object as this one.  This "
           "object has type " << this_name << " and its subclass of "
           "factor_output_type has type " << factor_output_type_name
           << ", but the input factor_output_type object has dynamic "
           "type " << dynamic_type_name << ".");
      }
      return *output_ptr;
    }

  public:
    void
    apply (const ApplyType& apply_type,
           const LocalOrdinal nrows,
           const LocalOrdinal ncols_Q,
           const Scalar Q[],
           const LocalOrdinal ldq,
           const factor_output_type& factor_output,
           const LocalOrdinal ncols_C,
           Scalar C[],
           const LocalOrdinal ldc,
           const bool contigCacheBlocks) const override
    {
      const char prefix[] = "TSQR::CuSolverNodeTsqr::apply: ";

      // Quick exit and error tests
      if (ncols_Q == 0 || ncols_C == 0 || nrows == 0) {
        return;
      }
      else if (ldc < nrows) {
        std::ostringstream os;
        os << prefix << "ldc (= " << ldc << ") < nrows (= "
           << nrows << ")";
        throw std::invalid_argument (os.str());
      }
      else if (ldq < nrows) {
        std::ostringstream os;
        os << prefix << "ldq (= " << ldq << ") < nrows (= "
           << nrows << ")";
        throw std::invalid_argument (os.str());
      }

      const char side = 'L';
      const char trans = apply_type.toString ()[0];
      auto tau = get_my_factor_output (factor_output).tau ();
      // FIXME (mfh 11 Dec 2019) TSQR::Impl::CuBlas takes
      // std::complex, but Kokkos::View stores Kokkos::complex.  We're
      // assuming they have the same alignment here, but all of Tpetra
      // assumes that.
      const Scalar* tau_raw =
        reinterpret_cast<const Scalar*> (tau.data ());
      auto work =
        get_work_for_apply_Q_factor (apply_type,
                                     nrows, ncols_C, ncols_Q,
                                     Q, ldq, tau_raw, C, ldc);
      Scalar* work_raw = reinterpret_cast<Scalar*> (work.data ());
      const int lwork (work.extent (0));
      auto info = get_info ();

      using TSQR::Impl::CuSolver;
      using TSQR::Impl::CuSolverHandle;
      CuSolver<Scalar> solver
        {CuSolverHandle::getSingleton (), info.data ()};
      solver.apply_Q_factor (side, trans,
                             nrows, ncols_C, ncols_Q,
                             Q, ldq, tau_raw,
                             C, ldc,
                             work_raw, lwork);
    }

    /// \brief Fill C (DEVICE MEMORY) with the first C.extent(1)
    ///   columns of the identity matrix.  Assume that C has already
    ///   been pre-filled with zeros.
    void
    fill_with_identity_columns
      (const MatView<LocalOrdinal, Scalar>& C) const override
    {
      auto C_view =
        Impl::get_device_mat_view (C.extent (0), C.extent (1),
                                   C.data (), C.stride (1));
      Impl::set_diagonal_entries_to_one (C_view);
    }

    void
    explicit_Q (const LocalOrdinal nrows,
                const LocalOrdinal ncols_Q,
                const Scalar Q[], // DEVICE MEMORY
                const LocalOrdinal ldq,
                const factor_output_type& factor_output,
                const LocalOrdinal ncols_C,
                Scalar C[], // DEVICE MEMORY
                const LocalOrdinal ldc,
                const bool contigCacheBlocks) const override
    {
      using Impl::get_device_mat_view;
      auto C_view = get_device_mat_view (nrows, ncols_C, C, ldc);
      using IST = Impl::non_const_kokkos_value_type<Scalar>;
      deep_copy (C_view, IST {});
      Impl::set_diagonal_entries_to_one (C_view);
      apply (ApplyType::NoTranspose,
             nrows, ncols_Q, Q, ldq, factor_output,
             ncols_C, C, ldc, contigCacheBlocks);
    }

    void
    Q_times_B (const LocalOrdinal nrows,
               const LocalOrdinal ncols,
               Scalar Q[], // DEVICE MEMORY
               const LocalOrdinal ldq,
               const Scalar B[], // HOST MEMORY
               const LocalOrdinal ldb,
               const bool /* contigCacheBlocks */) const override
    {
      // Take the easy exit if available.
      if (ncols == 0 || nrows == 0) {
        return;
      }

      // _GEMM doesn't permit the in/out matrix to alias either of the
      // two input matrices, so we must make a copy.
      auto Q_copy = get_Q_copy (nrows, ncols, Q, ldq);

      // We assume that B is in host memory, so we need to copy it to
      // device before we can use cuBLAS.
      auto B_copy = get_B_copy (ncols, B, ldb);

      constexpr Scalar ZERO {};
      constexpr Scalar ONE (1.0);

      using TSQR::Impl::CuBlas;
      using TSQR::Impl::CuBlasHandle;
      CuBlas<Scalar> blas {CuBlasHandle::getSingleton ()};

      const char transa = 'N';
      const char transb = 'N';
      // FIXME (mfh 11 Dec 2019) TSQR::Impl::CuBlas takes
      // std::complex, but Kokkos::View stores Kokkos::complex.  We're
      // assuming they have the same alignment here, but all of Tpetra
      // assumes that.
      const Scalar* Q_copy_raw =
        reinterpret_cast<const Scalar*> (Q_copy.data ());
      const int Q_copy_stride (Q_copy.stride (1));
      blas.gemm (transa, transb, nrows, ncols, ncols,
                 ONE, Q_copy_raw, Q_copy_stride,
                 B, ldb, ZERO, Q, ldq);
    }

    void
    cache_block (const LocalOrdinal /* nrows */,
                 const LocalOrdinal /* ncols */,
                 Scalar /* A_out */ [],
                 const Scalar /*A_in */ [],
                 const LocalOrdinal /* lda_in */) const override
    {}

    void
    un_cache_block (const LocalOrdinal /* nrows */,
                    const LocalOrdinal /* ncols */,
                    Scalar /* A_out */[],
                    const LocalOrdinal /* lda_out */,
                    const Scalar /* A_in */ []) const override
    {}

    void
    fill_with_zeros (const LocalOrdinal nrows,
                     const LocalOrdinal ncols,
                     Scalar A[],
                     const LocalOrdinal lda,
                     const bool /* contigCacheBlocks */) const override
    {
      auto A_view = Impl::get_device_mat_view (nrows, ncols, A, lda);
      Kokkos::deep_copy (A_view, kokkos_value_type {});
    }

  private:
    mutable tau_type tau_;
    mutable work_type work_;
    mutable Impl::info_type info_;
    mutable Impl::device_matrix_type<kokkos_value_type> R_copy_;
    mutable Impl::device_matrix_type<kokkos_value_type> Q_copy_;
    mutable Impl::device_matrix_type<kokkos_value_type> B_copy_;
  };

} // namespace TSQR

#endif // HAVE_TPETRATSQR_CUBLAS && HAVE_TPETRATSQR_CUSOLVER
#endif // TSQR_CUSOLVERNODETSQR_HPP
