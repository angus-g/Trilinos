// @HEADER
//
// ***********************************************************************
//
//   Zoltan2: A package of combinatorial algorithms for scientific computing
//                  Copyright 2012 Sandia Corporation
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
// Questions? Contact Karen Devine      (kddevin@sandia.gov)
//                    Erik Boman        (egboman@sandia.gov)
//                    Siva Rajamanickam (srajama@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#include <iostream>
#include <fstream>
#include <limits>
#include <vector>
#include <Teuchos_RCP.hpp>
#include <Teuchos_ParameterList.hpp>
#include <Teuchos_CommandLineProcessor.hpp>
#include <Tpetra_CrsMatrix.hpp>
#include <Tpetra_Vector.hpp>
#include <MatrixMarket_Tpetra.hpp>
#include <Zoltan2_XpetraCrsMatrixAdapter.hpp>
#include <Zoltan2_TestHelpers.hpp>
#include <Zoltan2_ColoringProblem.hpp>

using Teuchos::RCP;

/////////////////////////////////////////////////////////////////////////////
// Program to demonstrate use of Zoltan2 to color a TPetra matrix
// (read from a MatrixMarket file or generated by Galeri::Xpetra).
// We assume the matrix is structurally symmetric.
// Usage:
//     a.out [--inputFile=filename] [--outputFile=outfile] [--verbose]
//           [--x=#] [--y=#] [--z=#] [--matrix={Laplace1D,Laplace2D,Laplace3D}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Eventually want to use Teuchos unit tests to vary z2TestLO and
// GO.  For now, we set them at compile time based on whether Tpetra
// is built with explicit instantiation on.  (in Zoltan2_TestHelpers.hpp)

typedef zlno_t z2TestLO;
typedef zgno_t z2TestGO;
typedef zscalar_t z2TestScalar;

typedef Tpetra::CrsMatrix<z2TestScalar, z2TestLO, z2TestGO> SparseMatrix;
typedef Tpetra::Vector<z2TestScalar, z2TestLO, z2TestGO> Vector;
typedef Vector::node_type Node;

typedef Tpetra::Import<z2TestLO, z2TestGO> Import;

typedef Zoltan2::XpetraCrsMatrixAdapter<SparseMatrix> SparseMatrixAdapter;

int validateColoring(RCP<SparseMatrix> A, int *color)
// returns 0 if coloring is valid, nonzero if invalid
{
  int nconflicts = 0;
  Teuchos::ArrayView<const zlno_t> indices;
  Teuchos::ArrayView<const zscalar_t> values; // Not used

  // Count conflicts in the graph.
  // Loop over local rows, treat local column indices as edges.
  zlno_t n = A->getNodeNumRows();
  for (zlno_t i=0; i<n; i++) {
    A->getLocalRowView(i, indices, values);
    for (zlno_t j=0; j<indices.size(); j++) {
      if ((indices[j]<n) && (indices[j]!=i) && (color[i]==color[indices[j]])){
        nconflicts++;
        //std::cout << "Debug: found conflict (" << i << ", " << indices[j] << ")" << std::endl;
      }
    }
  }

  return nconflicts;
}

int validateDistributedColoring(RCP<SparseMatrix> A, int *color){
  int nconflicts = 0;
  RCP<const SparseMatrix::map_type> rowMap = A->getRowMap();
  RCP<const SparseMatrix::map_type> colMap = A->getColMap();
  Vector R = Vector(rowMap);
  //put colors in the scalar entries of R
  for(size_t i = 0; i < A->getNodeNumRows(); i++){
    R.replaceLocalValue(i, color[i]);
  }

  Vector C = Vector(colMap);
  Import imp = Import(rowMap, colMap);
  C.doImport(R, imp, Tpetra::REPLACE);

  Teuchos::ArrayView<const zlno_t> indices;
  Teuchos::ArrayView<const zscalar_t> values; // not used

  //count conflicts in the graph
  //loop over local rows, treat local column indices as edges
  size_t n = A->getNodeNumRows();
  auto colorData = C.getData();
  for(size_t i = 0; i < n; i++){
    A->getLocalRowView(i, indices, values);
    for(Teuchos_Ordinal j = 0; j < indices.size(); j++){
      if( (indices[j] != i) && (color[i] == colorData[indices[j]]) ){
        nconflicts++;
      }
    }
  }

  return nconflicts;
}

int validateDistributedDistance2Coloring(RCP<SparseMatrix> A, int* color){
  int nconflicts = 0;

  SparseMatrix S(*A);
  SparseMatrix B(*A);
  Tpetra::MatrixMatrix::Multiply(*A, false, B, false, S);

  RCP<const SparseMatrix::map_type> rowMap = S.getRowMap();
  RCP<const SparseMatrix::map_type> colMap = S.getColMap();
  Vector R = Vector(rowMap);
  //put the colors in the scalar entries of R.
  for(size_t i = 0; i < S.getNodeNumRows(); i++){
    R.replaceLocalValue(i, color[i]);
  }

  Vector C = Vector(colMap);
  Import imp = Import(rowMap, colMap);
  C.doImport(R, imp, Tpetra::REPLACE);

  Teuchos::ArrayView<const zlno_t> indices;
  Teuchos::ArrayView<const zscalar_t> values; //not used

  //count conflicts in the graph
  //loop over local rows, treat local column indices as edges
  size_t n = S.getNodeNumRows();
  auto colorData = C.getData();
  for(size_t i = 0; i < n; i++){
    S.getLocalRowView(i, indices, values);
    for(Teuchos_Ordinal j = 0; j < indices.size(); j++){
      if( (indices[j] != i) && (color[i] == colorData[indices[j]]) ){
        nconflicts++;
      }
    }
  }

  return nconflicts;
}

int checkBalance(zlno_t n, int *color)
// Check size of color classes
{
  // Find max color
  int maxColor = 0;
  for (zlno_t i=0; i<n; i++) {
    if (color[i] > maxColor) maxColor = color[i];
  }

  // Compute color class sizes
  Teuchos::Array<int> colorCount(maxColor+1);
  for (zlno_t i=0; i<n; i++) {
    colorCount[color[i]]++;
  }

  // Find min and max, excluding color 0.
  int smallest = 1;
  int largest  = 1;
  zlno_t small = colorCount[1];
  zlno_t large = colorCount[1];
  for (int i=1; i<=maxColor; i++){
    if (colorCount[i] < small){
      small = colorCount[i];
      smallest = i;
    }
    if (colorCount[i] > large){
      large = colorCount[i];
      largest = i;
    }
  }

  /*std::cout << "Color size[0:2] = " << colorCount[0] << ", " << colorCount[1] << ", " << colorCount[2] << std::endl;
  std::cout << "Largest color class = " << largest << " with " << colorCount[largest] << " vertices." << std::endl;
  std::cout << "Smallest color class = " << smallest << " with " << colorCount[smallest] << " vertices." << std::endl;*/

  return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int narg, char** arg)
{
  std::string inputFile = "";            // Matrix Market file to read
  std::string outputFile = "";           // Output file to write
  std::string colorAlg = "SerialGreedy"; // Default algorithm is the serial greedy
  bool verbose = false;                  // Verbosity of output
  int testReturn = 0;

  std::string prepartition = "";    	 // Call Zoltan2 partitioning to better distribute
  					 // the graph before coloring
  bool prepartition_rows = false;	 // True if we're prepartitioning wrt rows
  bool prepartition_nonzeros = false;    // True if prepartitioning wrt matrix nonzeros

  int localColors = 0;
  int totalColors = 0;
  ////// Establish session.
  Tpetra::ScopeGuard tscope(&narg, &arg);
  Teuchos::RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();
  int me = comm->getRank();

  // Read run-time options.
  Teuchos::CommandLineProcessor cmdp (false, false);
  cmdp.setOption("colorMethod", &colorAlg,
		  "Coloring algorithms supported: SerialGreedy, Hybrid, 2GL, D2");
  cmdp.setOption("inputFile", &inputFile,
                 "Name of a Matrix Market file in the data directory; "
                 "if not specified, a matrix will be generated by Galeri.");
  cmdp.setOption("outputFile", &outputFile,
                 "Name of file to write the coloring");
  cmdp.setOption("verbose", "quiet", &verbose,
                 "Print messages and results.");
  cmdp.setOption("prepartition", &prepartition,
		  "Partition the input graph for better initial distribution;"
		  "valid values are rows and nonzeros");
  std::cout << "Starting everything" << std::endl;

  //////////////////////////////////
  // Even with cmdp option "true", I get errors for having these
  //   arguments on the command line.  (On redsky build)
  // KDDKDD Should just be warnings, right?  Code should still work with these
  // KDDKDD params in the create-a-matrix file.  Better to have them where
  // KDDKDD they are used.
  int xdim=10;
  int ydim=10;
  int zdim=10;
  std::string matrixType("Laplace3D");

  cmdp.setOption("x", &xdim,
                "number of gridpoints in X dimension for "
                "mesh used to generate matrix.");
  cmdp.setOption("y", &ydim,
                "number of gridpoints in Y dimension for "
                "mesh used to generate matrix.");
  cmdp.setOption("z", &zdim,
                "number of gridpoints in Z dimension for "
                "mesh used to generate matrix.");
  cmdp.setOption("matrix", &matrixType,
                "Matrix type: Laplace1D, Laplace2D, or Laplace3D");

  //////////////////////////////////
  // Coloring options to test.
  //////////////////////////////////
  std::string colorMethod("FirstFit");
  //int balanceColors = 0;
  cmdp.setOption("color_choice", &colorMethod,
       "Color choice method: FirstFit, LeastUsed, Random, RandomFast");
  // cmdp.setOption("balance_colors", &balanceColors,
  //                "Balance the size of color classes: 0/1 for false/true");

  //////////////////////////////////
  cmdp.parse(narg, arg);

  if(prepartition != ""){
    if(prepartition == "rows") prepartition_rows = true;
    else if (prepartition == "nonzeros") prepartition_nonzeros = true;
    else {
      std::cout << "Invalid value of prepartition option " << prepartition
	        << std::endl;
      std::cout << "No prepartitioning will be done" <<std::endl;
    }
  }

  RCP<UserInputForTests> uinput;

  if (inputFile != ""){ // Input file specified; read a matrix
    uinput = rcp(new UserInputForTests(testDataFilePath, inputFile, comm, true));
  }
  else                  // Let Galeri generate a matrix

    uinput = rcp(new UserInputForTests(xdim, ydim, zdim, matrixType, comm, true, true));

  RCP<SparseMatrix> Matrix = uinput->getUITpetraCrsMatrix();

  if (me == 0)
    std::cout << "NumRows     = " << Matrix->getGlobalNumRows() << std::endl
         << "NumNonzeros = " << Matrix->getGlobalNumEntries() << std::endl
         << "NumProcs = " << comm->getSize() << std::endl;
  if(prepartition_rows || prepartition_nonzeros){
    std::cout<<comm->getRank() <<": Starting to pre-partition, creating adapter\n";
    //compute new partition of matrix
    std::unique_ptr<SparseMatrixAdapter> zadapter;
    if(prepartition_nonzeros){
      zadapter = std::unique_ptr<SparseMatrixAdapter>(new SparseMatrixAdapter(Matrix, 1));
      zadapter->setRowWeightIsNumberOfNonZeros(0);
    } else {
      zadapter = std::unique_ptr<SparseMatrixAdapter>(new SparseMatrixAdapter(Matrix));
    }

    std::cout<<comm->getRank()<<": created adapter, creating PartitioningProblem\n";
    Teuchos::ParameterList zparams;
    zparams.set("algorithm","pulp");
    zparams.set("imbalance_tolerance", 1.05);
    zparams.set("partitioning_approach","partition");
    Zoltan2::PartitioningProblem<SparseMatrixAdapter>
	     zproblem(zadapter.get(), &zparams);
    std::cout<<comm->getRank()<<": created PartitioningProblem, starting to solve\n";
    zproblem.solve();
    std::cout<<comm->getRank()<<": solved Partitioning Problem\n";
    //print partition characteristics before and after
    std::cout<<comm->getRank()<<": applying partition\n";
    typedef Zoltan2::EvaluatePartition<SparseMatrixAdapter> quality_t;
    quality_t evalbef(zadapter.get(), &zparams, comm, NULL);
    if(me == 0){
      std::cout<<"BEFORE PREPARTITION: Partition statistics:" << std::endl;
      evalbef.printMetrics(std::cout);
    }

    quality_t evalaft(zadapter.get(), &zparams, comm, &zproblem.getSolution());
    if(me == 0){
      std::cout<<"AFTER PREPARTITION: Partition statistics:"<<std::endl;
      evalaft.printMetrics(std::cout);
    }
    std::cout<<comm->getRank()<<": done evaluating, migrating matrix to use new partitioning\n";
    //Migrate matrix to the new partition
    RCP<SparseMatrix> newMatrix;
    zadapter->applyPartitioningSolution(*Matrix, newMatrix, zproblem.getSolution());
    
    std::cout<<comm->getRank()<<": done applying, replacing old matrix with new one\n";
    Matrix = newMatrix;
    std::cout<<comm->getRank()<<": done replacing, finished partitioning\n";
  }
  ////// Specify problem parameters
  Teuchos::ParameterList params;
  params.set("color_choice", colorMethod);
  params.set("color_method", colorAlg);
  params.set("verbose", verbose);
  //params.set("balance_colors", balanceColors); // TODO

  ////// Create an input adapter for the Tpetra matrix.
  SparseMatrixAdapter adapter(Matrix);

  ////// Create and solve ordering problem
  try
  {
  Zoltan2::ColoringProblem<SparseMatrixAdapter> problem(&adapter, &params);
  std::cout << "Going to color" << std::endl;
  problem.solve();

  ////// Basic metric checking of the coloring solution
  size_t checkLength;
  int *checkColoring = nullptr;

  Zoltan2::ColoringSolution<SparseMatrixAdapter> *soln = problem.getSolution();

  std::cout << "Going to get results" << std::endl;
  // Check that the solution is really a coloring
  checkLength = soln->getColorsSize();
  if(checkLength > 0){
    checkColoring = soln->getColors();
    localColors = soln->getNumColors();
  }

  //count the number of colors used globally
  Teuchos::reduceAll<int,int>(*comm, Teuchos::REDUCE_MAX, 1, &localColors, &totalColors);
  if (outputFile != "") {
    std::ofstream colorFile;

    // Write coloring to file,
    // each process writes local coloring to a separate file
    //std::string fname = outputFile + "." + me;
    std::stringstream fname;
    fname << outputFile << "." << comm->getSize() << "." << me;
    colorFile.open(fname.str().c_str());
    for (size_t i=0; i<checkLength; i++){
      colorFile << " " << checkColoring[i] << std::endl;
    }
    colorFile.close();
  }

  // Print # of colors on each proc.
  std::cout << "No. of colors on proc " << me << " : " << localColors << std::endl;

  std::cout << "Going to validate the soln" << std::endl;
  // Verify that checkColoring is a coloring
  if(colorAlg == "D2"){
    testReturn = validateDistributedDistance2Coloring(Matrix, checkColoring);
  }else if(colorAlg == "2GL" || colorAlg == "Hybrid"){
    testReturn = validateDistributedColoring(Matrix, checkColoring);
  } else if (checkLength > 0){
    testReturn = validateColoring(Matrix, checkColoring);
  }

  // Check balance (not part of pass/fail for now)
  if(checkLength > 0) checkBalance((zlno_t)checkLength, checkColoring);

  } catch (std::exception &e){
      std::cout << "Exception caught in coloring" << std::endl;
      std::cout << e.what() << std::endl;
      std::cout << "FAIL" << std::endl;
      return 0;
  }

  int numGlobalConflicts = 0;
  Teuchos::reduceAll<int, int>(*comm, Teuchos::REDUCE_MAX, 1, &testReturn, &numGlobalConflicts);

  if (me == 0) {
    if (numGlobalConflicts > 0){
      std::cout <<"Number of conflicts found = "<<numGlobalConflicts<<std::endl;
      std::cout << "Solution is not a valid coloring; FAIL" << std::endl;
    }else{
      std::cout << "Used " <<totalColors<<" colors\n";
      std::cout << "PASS" << std::endl;
    }
  }

}

