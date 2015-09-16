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

/*! \file AdapterForTests.hpp
 *  \brief Generate Adapter for testing purposes.
 */


#ifndef ADAPTERFORTESTS
#define ADAPTERFORTESTS

#include <Zoltan2_Parameters.hpp>
#include <UserInputForTests.hpp>

#include <Zoltan2_PartitioningProblem.hpp>
#include <Zoltan2_PartitioningSolutionQuality.hpp>

#include <Zoltan2_BasicIdentifierAdapter.hpp>
#include <Zoltan2_XpetraCrsGraphAdapter.hpp>
#include <Zoltan2_XpetraCrsMatrixAdapter.hpp>
#include <Zoltan2_XpetraMultiVectorAdapter.hpp>
#include <Zoltan2_BasicVectorAdapter.hpp>
#include <Zoltan2_PamgenMeshAdapter.hpp>

#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_XMLObject.hpp>
#include <Teuchos_FileInputSource.hpp>

#include <Tpetra_MultiVector.hpp>
#include <Tpetra_CrsMatrix.hpp>

#include <string>
#include <iostream>
#include <vector>

using namespace std;
using Teuchos::RCP;
using Teuchos::ArrayRCP;
using Teuchos::ArrayView;
using Teuchos::Array;
using Teuchos::Comm;
using Teuchos::rcp;
using Teuchos::arcp;
using Teuchos::rcp_const_cast;
using Teuchos::ParameterList;
using std::string;


class AdapterForTests{
public:
  
  typedef UserInputForTests::tcrsMatrix_t tcrsMatrix_t;
  typedef UserInputForTests::tcrsGraph_t tcrsGraph_t;
  typedef UserInputForTests::tVector_t tVector_t;
  typedef UserInputForTests::tMVector_t tMVector_t;
  
  typedef UserInputForTests::xcrsMatrix_t xcrsMatrix_t;
  typedef UserInputForTests::xcrsGraph_t xcrsGraph_t;
  typedef UserInputForTests::xVector_t xVector_t;
  typedef UserInputForTests::xMVector_t xMVector_t;
  
  typedef Zoltan2::BasicUserTypes<zscalar_t, zzgid_t, zlno_t, zgno_t> userTypes_t;
  typedef Zoltan2::BaseAdapter<userTypes_t> base_adapter_t;
  typedef Zoltan2::BasicIdentifierAdapter<userTypes_t> basic_id_t;
  typedef Zoltan2::XpetraMultiVectorAdapter<tMVector_t> xpetra_mv_adapter;
  typedef Zoltan2::XpetraCrsGraphAdapter<tcrsGraph_t, tMVector_t> xcrsGraph_adapter;
  typedef Zoltan2::XpetraCrsMatrixAdapter<tcrsMatrix_t, tMVector_t> xcrsMatrix_adapter;
  typedef Zoltan2::BasicVectorAdapter<tMVector_t> basic_vector_adapter;
  typedef Zoltan2::PamgenMeshAdapter<tMVector_t> pamgen_adapter_t;
  
  static base_adapter_t* getAdapterForInput(UserInputForTests *uinput, const ParameterList &pList, const RCP<const Comm<int> > &comm);
  
private:
  /*! \brief Method to choose and call the correct constructor
   *      for a BasicIdentifierAdapter from a UserInputForTests input file.
   *   \param uinput is the UserInputForTestsInputForTestObject
   *   \param  pList is the teuchos input parameter list
   *   \param  adapter is a reference to the input adapter to be constructed.
   */
  static base_adapter_t*
  getBasicIdentiferAdapterForInput(UserInputForTests *uinput, const ParameterList &pList, const RCP<const Comm<int> > &comm);
  
  /*! \brief Method to choose and call the correct constructor
   *      for a XpetraMultiVectorAdapter from a UserInputForTests input file.
   *   \param uinput is the UserInputForTestsInputForTestObject
   *   \param  pList is the teuchos input parameter list
   *   \param  adapter is a reference to the input adapter to be constructed.
   */
  static base_adapter_t*
  getXpetraMVAdapterForInput(UserInputForTests *uinput, const ParameterList &pList, const RCP<const Comm<int> > &comm);
  
  /*! \brief Method to choose and call the correct constructor
   *      for a XpetraMultiVectorAdapter from a UserInputForTests input file.
   *   \param uinput is the UserInputForTestsInputForTestObject
   *   \param  pList is the teuchos input parameter list
   *   \param  adapter is a reference to the input adapter to be constructed.
   */
  static base_adapter_t*
  getXpetraCrsGraphAdapterForInput(UserInputForTests *uinput, const ParameterList &pList, const RCP<const Comm<int> > &comm);
  
  /*! \brief Method to choose and call the correct constructor
   *      for a XpetraMultiVectorAdapter from a UserInputForTests input file.
   *   \param uinput is the UserInputForTestsInputForTestObject
   *   \param  pList is the teuchos input parameter list
   *   \param  adapter is a reference to the input adapter to be constructed.
   */
  static base_adapter_t*
  getXpetraCrsMatrixAdapterForInput(UserInputForTests *uinput, const ParameterList &pList, const RCP<const Comm<int> > &comm);
  
  /*! \brief Method to choose and call the correct constructor
   *      for a BasicVectorAdapter from a UserInputForTests input file.
   *   \param uinput is the UserInputForTestsInputForTestObject
   *   \param  pList is the teuchos input parameter list
   *   \param  adapter is a reference to the input adapter to be constructed.
   */
  static base_adapter_t*
  getBasicVectorAdapterForInput(UserInputForTests *uinput, const ParameterList &pList, const RCP<const Comm<int> > &comm);
  
  
  static base_adapter_t*
  getPamgenMeshAdapterForInput(UserInputForTests *uinput, const ParameterList &pList, const RCP<const Comm<int> > &comm);
  /*! \brief Method to set up strided vector data from a Multivector
   *  \param data is the Multivector
   *  \param coords is the vector of strided data
   *  \param strides is the vector of stride info
   *  \param stride is stride to apply to data set
   * \param epetra tells the method whether an epetra vector is being passed
   */
  template <typename T>
  static void InitializeVectorData(const RCP<T> &data,
                                   vector<const zscalar_t *> &coords,
                                   vector<int> & strides,
                                   int stride);
  
#ifdef HAVE_EPETRA_DATA_TYPES
  template <typename T>
  static void InitializeEpetraVectorData(const RCP<T> &data,
                                         vector<const zscalar_t *> &coords,
                                         vector<int> & strides,
                                         int stride);
#endif
};


AdapterForTests::base_adapter_t * AdapterForTests::getAdapterForInput(UserInputForTests *uinput,
                                                                      const ParameterList &pList,
                                                                      const RCP<const Comm<int> > &comm)
{
  
  if(!pList.isParameter("input adapter"))
    throw std::runtime_error("Input adapter not specified");
  
  // pick method for chosen adapter
  string adapter_name = pList.get<string>("input adapter");
  AdapterForTests::base_adapter_t * ia = nullptr; // input adapter
  if(adapter_name == "BasicIdentifier")
    ia = AdapterForTests::getBasicIdentiferAdapterForInput(uinput, pList, comm);
  else if(adapter_name == "XpetraMultiVector")
    ia = AdapterForTests::getXpetraMVAdapterForInput(uinput, pList, comm);
  else if(adapter_name == "XpetraCrsGraph")
    ia = getXpetraCrsGraphAdapterForInput(uinput,pList, comm);
  else if(adapter_name == "XpetraCrsMatrix")
    ia = getXpetraCrsMatrixAdapterForInput(uinput,pList, comm);
  else if(adapter_name == "BasicVector")
    ia = getBasicVectorAdapterForInput(uinput,pList, comm);
  else if(adapter_name == "PamgenMesh")
    ia = getPamgenMeshAdapterForInput(uinput,pList, comm);
  else
    throw std::runtime_error("Input adapter type not available, or misspelled.");
  
  return ia;
}


AdapterForTests::base_adapter_t * AdapterForTests::getBasicIdentiferAdapterForInput(UserInputForTests *uinput,
                                                                                    const ParameterList &pList,
                                                                                    const RCP<const Comm<int> > &comm)
{
  
  if(!pList.isParameter("data type"))
    throw std::runtime_error("Input data type not specified");
  
  string input_type = pList.get<string>("data type"); // get the input type
  
  if (!uinput->hasInputDataType(input_type))
    throw std::runtime_error("Input type:" + input_type + ", not available or misspelled."); // bad type
  
  vector<const zscalar_t *> weights;
  std::vector<int> weightStrides;
  const zzgid_t * globalIds;
  size_t localCount = 0;
  
  // get weights if any
  // get weights if any
  if(uinput->hasUIWeights())
  {
    RCP<tMVector_t> vtx_weights = uinput->getUIWeights();
    // copy to weight
    size_t cols = vtx_weights->getNumVectors();
    for (size_t i = 0; i< cols; i++) {
      weights.push_back(vtx_weights->getData(i).getRawPtr());
      weightStrides.push_back((int)vtx_weights->getStride());
    }
  }
  
  if(input_type == "coordinates")
  {
    RCP<tMVector_t> data = uinput->getUICoordinates();
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getLocalLength();
  }
  else if(input_type == "tpetra_vector")
  {
    RCP<tVector_t> data = uinput->getUITpetraVector();
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getLocalLength();
  }
  else if(input_type == "tpetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<tMVector_t> data = uinput->getUITpetraMultiVector(nvec);
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getLocalLength();
  }
  else if(input_type == "tpetra_crs_graph")
  {
    RCP<tcrsGraph_t> data = uinput->getUITpetraCrsGraph();
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getNodeNumCols();
  }
  else if(input_type == "tpetra_crs_matrix")
  {
    RCP<tcrsMatrix_t> data = uinput->getUITpetraCrsMatrix();
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getNodeNumCols();
  }
  else if(input_type == "xpetra_vector")
  {
    RCP<xVector_t> data = uinput->getUIXpetraVector();
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getLocalLength();
  }
  else if(input_type == "xpetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<xMVector_t> data = uinput->getUIXpetraMultiVector(nvec);
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getLocalLength();
  }
  else if(input_type == "xpetra_crs_graph")
  {
    RCP<xcrsGraph_t> data = uinput->getUIXpetraCrsGraph();
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getNodeNumCols();
  }
  else if(input_type == "xpetra_crs_matrix")
  {
    RCP<xcrsMatrix_t> data = uinput->getUIXpetraCrsMatrix();
    globalIds = (zzgid_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = data->getNodeNumCols();
  }
#ifdef HAVE_EPETRA_DATA_TYPES
  else if(input_type == "epetra_vector")
  {
    RCP<Epetra_Vector> data = uinput->getUIEpetraVector();
    globalIds = (zzgid_t *)data->Map().MyGlobalElements();
    localCount = data->MyLength();
  }
  else if(input_type == "epetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<Epetra_MultiVector> data = uinput->getUIEpetraMultiVector(nvec);
    globalIds = (zzgid_t *)data->Map().MyGlobalElements();
    localCount = data->MyLength();
  }
  else if(input_type == "epetra_crs_graph")
  {
    RCP<Epetra_CrsGraph> data = uinput->getUIEpetraCrsGraph();
    globalIds = (zzgid_t *)data->Map().MyGlobalElements();
    localCount = data->NumMyCols();
  }
  else if(input_type == "epetra_crs_matrix")
  {
    RCP<Epetra_CrsMatrix> data = uinput->getUIEpetraCrsMatrix();
    globalIds = (zzgid_t *)data->Map().MyGlobalElements();
    localCount = data->NumMyCols();
  }
#endif
  
  if(localCount == 0) return nullptr;
  return reinterpret_cast<AdapterForTests::base_adapter_t *>( new AdapterForTests::basic_id_t(zlno_t(localCount),globalIds,weights,weightStrides));
}


AdapterForTests::base_adapter_t * AdapterForTests::getXpetraMVAdapterForInput(UserInputForTests *uinput,
                                                                              const ParameterList &pList,
                                                                              const RCP<const Comm<int> > &comm)
{
  
  if(!pList.isParameter("data type"))
    throw std::runtime_error("Input data type not specified");
  
  string input_type = pList.get<string>("data type");
  if (!uinput->hasInputDataType(input_type))
    throw std::runtime_error("Input type:" + input_type + ", not available or misspelled."); // bad type
  
  AdapterForTests::base_adapter_t * adapter = nullptr;
  vector<const zscalar_t *> weights;
  std::vector<int> weightStrides;
  
  // get weights if any
  if(uinput->hasUIWeights())
  {
    RCP<tMVector_t> vtx_weights = uinput->getUIWeights();
    // copy to weight
    size_t weightsPerRow = vtx_weights->getNumVectors();
    for (size_t i = 0; i< weightsPerRow; i++) {
      weights.push_back(vtx_weights->getData(i).getRawPtr());
      weightStrides.push_back(1);
    }
  }
  
  // set adapter
  if(input_type == "coordinates")
  {
    RCP<tMVector_t> data = uinput->getUICoordinates();
    RCP<const tMVector_t> const_data = rcp_const_cast<const tMVector_t>(data);
    if(weights.empty())
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(new Zoltan2::XpetraMultiVectorAdapter<tMVector_t>(const_data));
    else
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(new Zoltan2::XpetraMultiVectorAdapter<tMVector_t>(const_data,weights,weightStrides));
  }
  else if(input_type == "tpetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<tMVector_t> data = uinput->getUITpetraMultiVector(nvec);
    RCP<const tMVector_t> const_data = rcp_const_cast<const tMVector_t>(data);
    if(weights.empty())
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(new Zoltan2::XpetraMultiVectorAdapter<tMVector_t>(const_data));
    else
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(new Zoltan2::XpetraMultiVectorAdapter<tMVector_t>(const_data,weights,weightStrides));
  }
  else if(input_type == "xpetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<xMVector_t> data = uinput->getUIXpetraMultiVector(nvec);
    RCP<const xMVector_t> const_data = rcp_const_cast<const xMVector_t>(data);
    if(weights.empty())
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(new Zoltan2::XpetraMultiVectorAdapter<xMVector_t>(const_data));
    else{
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(new Zoltan2::XpetraMultiVectorAdapter<xMVector_t>(const_data,weights,weightStrides));
    }
  }
#ifdef HAVE_EPETRA_DATA_TYPES
  
  else if(input_type == "epetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<Epetra_MultiVector> data = uinput->getUIEpetraMultiVector(nvec);
    RCP<const Epetra_MultiVector> const_data = rcp_const_cast<const Epetra_MultiVector>(data);
    
    if(weights.empty())
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(
                                                                    new Zoltan2::XpetraMultiVectorAdapter<Epetra_MultiVector>(const_data));
    else
      adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(
                                                                    new Zoltan2::XpetraMultiVectorAdapter<Epetra_MultiVector>(const_data,weights,weightStrides));
  }
#endif
  
  if(adapter == nullptr)
    throw std::runtime_error("Input data chosen not compatible with xpetra multi-vector adapter.");
  else
    return adapter;
}


AdapterForTests::base_adapter_t * AdapterForTests::getXpetraCrsGraphAdapterForInput(UserInputForTests *uinput,
                                                                                    const ParameterList &pList,
                                                                                    const RCP<const Comm<int> > &comm)
{
  if(!pList.isParameter("data type"))
    throw std::runtime_error("Input data type not specified");
  
  string input_type = pList.get<string>("data type");
  if (!uinput->hasInputDataType(input_type))
    throw std::runtime_error("Input type:" + input_type + ", not available or misspelled."); // bad type
  
  
  AdapterForTests::base_adapter_t * adapter = nullptr;
  vector<const zscalar_t *> vtx_weights;
  vector<const zscalar_t *> edge_weights;
  vector<int> vtx_weightStride;
  vector<int> edge_weightStride;
  
  // get vtx weights if any
  if(uinput->hasUIWeights())
  {
    RCP<tMVector_t> vtx_weights_tmp = uinput->getUIWeights();
    // copy to weight
    size_t weightsPerRow = vtx_weights_tmp->getNumVectors();
    for (size_t i = 0; i< weightsPerRow; i++) {
      vtx_weights.push_back(vtx_weights_tmp->getData(i).getRawPtr());
      vtx_weightStride.push_back(1);
    }
  }
  
  // get edge weights if any
  if(uinput->hasUIEdgeWeights())
  {
    RCP<tMVector_t> edge_weights_tmp = uinput->getUIEdgeWeights();
    // copy to weight
    size_t weightsPerRow = edge_weights_tmp->getNumVectors();
    for (size_t i = 0; i< weightsPerRow; i++) {
      edge_weights.push_back(edge_weights_tmp->getData(i).getRawPtr());
      edge_weightStride.push_back(1);
    }
  }
  
  
  // set adapter
  if(input_type == "tpetra_crs_graph")
  {
    typedef Zoltan2::XpetraCrsGraphAdapter<tcrsGraph_t, tMVector_t> problem_t;
    
    RCP<tcrsGraph_t> data = uinput->getUITpetraCrsGraph();
    RCP<const tcrsGraph_t> const_data = rcp_const_cast<const tcrsGraph_t>(data);
    problem_t *ia = new problem_t(const_data,(int)vtx_weights.size(),(int)edge_weights.size());
    
    if(!vtx_weights.empty())
    {
      for(int i = 0; i < (int)vtx_weights.size(); i++)
        ia->setVertexWeights(vtx_weights[i],vtx_weightStride[i],i);
    }
    
    if(!edge_weights.empty())
    {
      for(int i = 0; i < (int)edge_weights.size(); i++)
        ia->setEdgeWeights(edge_weights[i],edge_weightStride[i],i);
    }
    
    adapter =  reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
  }
  else if(input_type == "xpetra_crs_graph")
  {
    typedef Zoltan2::XpetraCrsGraphAdapter<xcrsGraph_t, tMVector_t> problem_t;
    
    RCP<xcrsGraph_t> data = uinput->getUIXpetraCrsGraph();
    RCP<const xcrsGraph_t> const_data = rcp_const_cast<const xcrsGraph_t>(data);
    problem_t *ia = new problem_t(const_data, (int)vtx_weights.size(), (int)edge_weights.size());
    
    if(!vtx_weights.empty())
    {
      for(int i = 0; i < (int)vtx_weights.size(); i++)
        ia->setVertexWeights(vtx_weights[i],vtx_weightStride[i],i);
    }
    
    if(!edge_weights.empty())
    {
      for(int i = 0; i < (int)edge_weights.size(); i++)
        ia->setEdgeWeights(edge_weights[i],edge_weightStride[i],i);
    }
    
    adapter =  reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
  }
#ifdef HAVE_EPETRA_DATA_TYPES
  
  else if(input_type == "epetra_crs_graph")
  {
    typedef Zoltan2::XpetraCrsGraphAdapter<Epetra_CrsGraph, tMVector_t> problem_t;
    
    RCP<Epetra_CrsGraph> data = uinput->getUIEpetraCrsGraph();
    RCP<const Epetra_CrsGraph> const_data = rcp_const_cast<const Epetra_CrsGraph>(data);
    problem_t *ia = new problem_t(const_data,(int)vtx_weights.size(),(int)edge_weights.size());
    
    if(!vtx_weights.empty())
    {
      for(int i = 0; i < (int)vtx_weights.size(); i++)
        ia->setVertexWeights(vtx_weights[i],vtx_weightStride[i],i);
    }
    
    if(!edge_weights.empty())
    {
      for(int i = 0; i < (int)edge_weights.size(); i++)
        ia->setEdgeWeights(edge_weights[i],edge_weightStride[i],i);
    }
    
    adapter =  reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
    
  }
#endif
  
  if(adapter == nullptr)
    throw std::runtime_error("Input data chosen not compatible with xpetra multi-vector adapter.");
  else{
    // make the coordinate adapter
    // get an adapter for the coordinates
    // need to make a copy of the plist and change the vector type
    Teuchos::ParameterList pCopy(pList);
    pCopy = pCopy.set<std::string>("data type","coordinates");
    
    AdapterForTests::base_adapter_t * ca = nullptr;
    ca = getXpetraMVAdapterForInput(uinput,pCopy, comm);
    
    if(ca == nullptr)
      throw std::runtime_error("Failed to create coordinate vector adapter for xpetra crs-matrix adapter.");
    
    // set the coordinate adapter
    reinterpret_cast<AdapterForTests::xcrsGraph_adapter *>(adapter)->setCoordinateInput(reinterpret_cast<AdapterForTests::xpetra_mv_adapter *>(ca));
    return adapter;
  }
  
}


AdapterForTests::base_adapter_t * AdapterForTests::getXpetraCrsMatrixAdapterForInput(UserInputForTests *uinput,
                                                                                     const ParameterList &pList,
                                                                                     const RCP<const Comm<int> > &comm)
{
  if(!pList.isParameter("data type"))
    throw std::runtime_error("Input data type not specified");
  
  string input_type = pList.get<string>("data type");
  if (!uinput->hasInputDataType(input_type))
    throw std::runtime_error("Input type:" + input_type + ", not available or misspelled."); // bad type
  
  AdapterForTests::base_adapter_t * adapter = nullptr;
  vector<const zscalar_t *> weights;
  vector<int> strides;
  
  // get weights if any
  if(uinput->hasUIWeights())
  {
    if(comm->getRank() == 0) cout << "Have weights...." << endl;
    RCP<tMVector_t> vtx_weights = uinput->getUIWeights();
    
    // copy to weight
    int weightsPerRow = (int)vtx_weights->getNumVectors();
    for (size_t i = 0; i< weightsPerRow; i++)
    {
      weights.push_back(vtx_weights->getData(i).getRawPtr());
      strides.push_back(1);
    }
    
  }
  
  // set adapter
  if(input_type == "tpetra_crs_matrix")
  {
    if(comm->getRank() == 0) cout << "Make tpetra crs matrix adapter...." << endl;
    
    // get pointer to data
    RCP<tcrsMatrix_t> data = uinput->getUITpetraCrsMatrix();
    RCP<const tcrsMatrix_t> const_data = rcp_const_cast<const tcrsMatrix_t>(data); // const cast data
    
    // new adapter
    xcrsMatrix_adapter *ia = new xcrsMatrix_adapter(const_data, (int)weights.size());
    
    // if we have weights set them
    if(!weights.empty())
    {
      for(int i = 0; i < (int)weights.size(); i++)
        ia->setWeights(weights[i],strides[i],i);
    }
    
    // cast to base type
    adapter = reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
    
  }
  else if(input_type == "xpetra_crs_matrix")
  {
    // type def this adapter type
    typedef Zoltan2::XpetraCrsMatrixAdapter<xcrsMatrix_t, tMVector_t> problem_t;
    
    RCP<xcrsMatrix_t> data = uinput->getUIXpetraCrsMatrix();
    RCP<const xcrsMatrix_t> const_data = rcp_const_cast<const xcrsMatrix_t>(data);
    
    // new adapter
    problem_t *ia = new problem_t(const_data, (int)weights.size());
    
    // if we have weights set them
    if(!weights.empty())
    {
      for(int i = 0; i < (int)weights.size(); i++)
         ia->setWeights(weights[i],strides[i],i);
    }
    
    adapter =  reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
    
  }
#ifdef HAVE_EPETRA_DATA_TYPES
  
  else if(input_type == "epetra_crs_matrix")
  {
    typedef Zoltan2::XpetraCrsMatrixAdapter<Epetra_CrsMatrix, tMVector_t> problem_t;
    
    RCP<Epetra_CrsMatrix> data = uinput->getUIEpetraCrsMatrix();
    RCP<const Epetra_CrsMatrix> const_data = rcp_const_cast<const Epetra_CrsMatrix>(data);
    
    // new adapter
    problem_t *ia = new problem_t(const_data, (int)weights.size());
    
    // if we have weights set them
    if(!weights.empty())
    {
      for(int i = 0; i < (int)weights.size(); i++)
         ia->setWeights(weights[i],strides[i],i);
    }
    
    adapter =  reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
  }
#endif
  
  if(adapter == nullptr)
    throw std::runtime_error("Input data chosen not compatible with xpetra crs-matrix adapter.");
  else{
    
    // make the coordinate adapter
    // get an adapter for the coordinates
    // need to make a copy of the plist and change the vector type
    Teuchos::ParameterList pCopy(pList);
    pCopy = pCopy.set<std::string>("data type","coordinates");
//    pCopy = pCopy.set<int>("vector_dimension", 1); // what is the proper value!?
    
    AdapterForTests::base_adapter_t * ca = nullptr;
    ca = getXpetraMVAdapterForInput(uinput,pCopy,comm);
    
    if(ca == nullptr)
      throw std::runtime_error("Failed to create coordinate vector adapter for xpetra crs-matrix adapter.");
    
    // set the coordinate adapter
    reinterpret_cast<AdapterForTests::xcrsMatrix_adapter *>(adapter)->setCoordinateInput(reinterpret_cast<AdapterForTests::xpetra_mv_adapter *>(ca));
    return adapter;
  }
  
}


AdapterForTests::base_adapter_t * AdapterForTests::getBasicVectorAdapterForInput(UserInputForTests *uinput,
                                                                                 const ParameterList &pList,
                                                                                 const RCP<const Comm<int> > &comm)
{
  if(!pList.isParameter("data type"))
    throw std::runtime_error("Input data type not specified");
  
  string input_type = pList.get<string>("data type");
  if (!uinput->hasInputDataType(input_type))
    throw std::runtime_error("Input type:" + input_type + ", not available or misspelled."); // bad type
  
  AdapterForTests::basic_vector_adapter * ia = nullptr; // pointer for basic vector adapter
  
  vector<const zscalar_t *> weights;
  std::vector<int> weightStrides;
  const zgno_t * globalIds;
  zlno_t localCount = 0;
  
  // get weights if any
  // get weights if any
  if(uinput->hasUIWeights())
  {
    RCP<tMVector_t> vtx_weights = uinput->getUIWeights();
    // copy to weight
    size_t cols = vtx_weights->getNumVectors();
    for (size_t i = 0; i< cols; i++) {
      weights.push_back(vtx_weights->getData(i).getRawPtr());
      weightStrides.push_back(1);
    }
  }
  
  // get vector stride
  int stride = 1;
  if(pList.isParameter("stride"))
    stride = pList.get<int>("stride");
  
  if(input_type == "coordinates")
  {
    RCP<tMVector_t> data = uinput->getUICoordinates();
    globalIds = (zgno_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = static_cast<zlno_t>(data->getLocalLength());
    
    // get strided data
    vector<const zscalar_t *> coords;
    vector<int> entry_strides;
    AdapterForTests::InitializeVectorData(data,coords,entry_strides,stride);
    
    size_t dim = data->getNumVectors();
    if(dim == 1) coords[1] = coords[2] = NULL;
    else if(dim == 2)
    {
      if(comm->getRank() == 0) cout << "2D setting coords 2 to nullll" << endl;
      coords[2] = NULL;
    }
    
    if(weights.empty())
    {
      ia = new AdapterForTests::basic_vector_adapter(zlno_t(localCount),
                                                     globalIds,
                                                     coords[0],coords[1],coords[2],
                                                     stride, stride, stride);
    }else{
      ia = new AdapterForTests::basic_vector_adapter(zlno_t(localCount),
                                                     globalIds,
                                                     coords[0],coords[1],coords[2],
                                                     stride, stride, stride,
                                                     true,
                                                     weights[0],
                                                     weightStrides[0]);
    }
    
  }
  else if(input_type == "tpetra_vector")
  {
    RCP<tVector_t> data = uinput->getUITpetraVector();
    globalIds = (zgno_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = static_cast<zlno_t>(data->getLocalLength());
    
    // get strided data
    vector<const zscalar_t *> coords;
    vector<int> entry_strides;
    AdapterForTests::InitializeVectorData(data,coords,entry_strides,stride);
    
    if(weights.empty())
    {
      ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                     coords[0], entry_strides[0]);
    }else{
      ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                     coords[0], entry_strides[0],
                                                     true,
                                                     weights[0],
                                                     weightStrides[0]);
      
    }
    
  }
  else if(input_type == "tpetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    
    RCP<tMVector_t> data = uinput->getUITpetraMultiVector(nvec);
    globalIds = (zgno_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = static_cast<zlno_t>(data->getLocalLength());
    
    // get strided data
    vector<const zscalar_t *> coords;
    vector<int> entry_strides;
    AdapterForTests::InitializeVectorData(data,coords,entry_strides,stride);
    
    ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                   coords, entry_strides,
                                                   weights,weightStrides);
    
  }
  else if(input_type == "xpetra_vector")
  {
    RCP<xVector_t> data = uinput->getUIXpetraVector();
    globalIds = (zgno_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = static_cast<zlno_t>(data->getLocalLength());
    
    // get strided data
    vector<const zscalar_t *> coords;
    vector<int> entry_strides;
    AdapterForTests::InitializeVectorData(data,coords,entry_strides,stride);
    
    if(weights.empty())
    {
      ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                     coords[0], entry_strides[0]);
    }else{
      ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                     coords[0], entry_strides[0],
                                                     true,
                                                     weights[0],
                                                     weightStrides[0]);
      
    }
  }
  else if(input_type == "xpetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<xMVector_t> data = uinput->getUIXpetraMultiVector(nvec);
    globalIds = (zgno_t *)data->getMap()->getNodeElementList().getRawPtr();
    localCount = static_cast<zlno_t>(data->getLocalLength());
    
    // get strided data
    vector<const zscalar_t *> coords;
    vector<int> entry_strides;
    AdapterForTests::InitializeVectorData(data,coords,entry_strides,stride);
    
    // make vector!
    ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                   coords, entry_strides,
                                                   weights,weightStrides);
  }
  
#ifdef HAVE_EPETRA_DATA_TYPES
  else if(input_type == "epetra_vector")
  {
    RCP<Epetra_Vector> data = uinput->getUIEpetraVector();
    globalIds = (zgno_t *)data->Map().MyGlobalElements();
    localCount = static_cast<zlno_t>(data->MyLength());
    
    // get strided data
    vector<const zscalar_t *> coords;
    vector<int> entry_strides;
    AdapterForTests::InitializeEpetraVectorData(data,coords,entry_strides,stride);
    
    if(weights.empty())
    {
      ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                     coords[0], entry_strides[0]);
    }else{
      ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                     coords[0], entry_strides[0],
                                                     true,
                                                     weights[0],
                                                     weightStrides[0]);
      
    }
    
    //    delete [] epetravectors;
  }
  else if(input_type == "epetra_multivector")
  {
    int nvec = pList.get<int>("vector_dimension");
    RCP<Epetra_MultiVector> data = uinput->getUIEpetraMultiVector(nvec);
    globalIds = (zgno_t *)data->Map().MyGlobalElements();
    localCount = data->MyLength();
    
    vector<const zscalar_t *> coords;
    vector<int> entry_strides;
    AdapterForTests::InitializeEpetraVectorData(data,coords,entry_strides,stride);
    
    // make vector!
    ia = new AdapterForTests::basic_vector_adapter(localCount, globalIds,
                                                   coords, entry_strides,
                                                   weights,weightStrides);
    
  }
  
#endif
  
  if(localCount == 0){
    if(ia != nullptr) delete ia;
    return nullptr;
  }
  return reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
  
}

template <typename T>
void AdapterForTests::InitializeVectorData(const RCP<T> &data,
                                           vector<const zscalar_t *> &coords,
                                           vector<int> & strides,
                                           int stride)
{
  // set up adapter data
  const zlno_t localCount = data->getLocalLength();
  const zlno_t nvecs = data->getNumVectors();
  const zlno_t vecsize = data->getNumVectors() * data->getLocalLength();
//    printf("Number of vectors by data: %zu\n", nvecs);
  //  printf("Size of data: %zu\n", vecsize);
  
  ArrayRCP<zscalar_t> *petravectors =
  new ArrayRCP<zscalar_t>[nvecs];
  
  //  printf("Getting t-petra vectors...\n");
  for (size_t i = 0; i < nvecs; i++)
    petravectors[i] = data->getDataNonConst(i);
  
  // debugging
  //  for (size_t i = 0; i < nvecs; i++){
  //    printf("Tpetra vector %zu: {",i);
  //
  //    for (size_t j = 0; j < localCount; j++)
  //    {
  //      printf("%1.2g ",petravectors[i][j]);
  //    }
  //    printf("}\n");
  //  }
  
  zlno_t idx = 0;
  zscalar_t *coordarr = new zscalar_t[vecsize];
  
  if(stride == 1 || stride != nvecs)
  {
    for (zlno_t i = 0; i < nvecs; i++) {
      for (zlno_t j = 0; j < localCount; j++) {
        coordarr[idx++] = petravectors[i][j];
      }
    }
  }else
  {
    for (zlno_t j = 0; j < localCount; j++) {
      for (zlno_t i = 0; i < nvecs; i++) {
        coordarr[idx++] = petravectors[i][j];
      }
    }
  }
  
  // debugging
  //  printf("Made coordarr : {");
  //  for (zlno_t i = 0; i < vecsize; i++){
  //    printf("%1.2g ",coordarr[i]);
  //  }
  //  printf("}\n");
  
  // always build for dim 3
  coords = std::vector<const zscalar_t *>(3);
  strides = std::vector<int>(nvecs);
  
  for (size_t i = 0; i < nvecs; i++) {
    if(stride == 1)
      coords[i] = &coordarr[i*localCount];
    else
      coords[i] = &coordarr[i];
    
    strides[i] = stride;
  }
  
  // debugging
  //  printf("Made coords...\n");
  //  for (size_t i = 0; i < nvecs; i++){
  //    const zscalar_t * tmp = coords[i];
  //    printf("coord %zu: {",i);
  //    for(size_t j = 0; j < localCount; j++)
  //    {
  //      printf("%1.2g ", tmp[j]);
  //    }
  //    printf("}\n");
  //  }
  
  //  printf("clean up coordarr and tpetravectors...\n\n\n");
  delete [] petravectors;
}

#ifdef HAVE_EPETRA_DATA_TYPES

template <typename T>
void AdapterForTests::InitializeEpetraVectorData(const RCP<T> &data,
                                                 vector<const zscalar_t *> &coords,
                                                 vector<int> & strides,
                                                 int stride){
  const size_t localCount = data->MyLength();
  const size_t nvecs = data->NumVectors();
  const size_t vecsize = nvecs * localCount;
  
  //  printf("Number of vectors by data: %zu\n", nvecs);
  //  printf("Size of data: %zu\n", vecsize);
  
  vector<zscalar_t *> epetravectors(nvecs);
  zscalar_t ** arr;
  //  printf("get data from epetra vector..\n");
  data->ExtractView(&arr);
  
  for(size_t k = 0; k < nvecs; k++)
  {
    epetravectors[k] = arr[k];
  }
  
  int idx = 0;
  basic_vector_adapter::scalar_t *coordarr = new basic_vector_adapter::scalar_t[vecsize];
  
  if(stride == 1 || stride != nvecs)
  {
    for (zlno_t i = 0; i < nvecs; i++) {
      for (zlno_t j = 0; j < localCount; j++) {
        coordarr[idx++] = epetravectors[i][j];
      }
    }
  }else
  {
    for (zlno_t j = 0; j < localCount; j++) {
      for (zlno_t i = 0; i < nvecs; i++) {
        coordarr[idx++] = epetravectors[i][j];
      }
    }
  }
  
  // debugging
//  printf("Made coordarr : {");
//  for (zlno_t i = 0; i < vecsize; i++){
//    printf("%1.2g ",coordarr[i]);
//  }
//  printf("}\n");
  
  coords = std::vector<const zscalar_t *>(3);
  strides = std::vector<int>(nvecs);
  
  for (size_t i = 0; i < nvecs; i++) {
    if(stride == 1)
      coords[i] = &coordarr[i*localCount];
    else
      coords[i] = &coordarr[i];
    
    strides[i] = stride;
  }
  
//  printf("Made coords...\n");
//  for (size_t i = 0; i < nvecs; i++){
//    const zscalar_t * tmp = coords[i];
//    printf("coord %zu: {",i);
//    for(size_t j = 0; j < localCount; j++)
//    {
//      printf("%1.2g ", tmp[j]);
//    }
//    printf("}\n");
//  }
  
}
#endif


// pamgen adapter
AdapterForTests::base_adapter_t*
AdapterForTests::getPamgenMeshAdapterForInput(UserInputForTests *uinput,
                                              const ParameterList &pList,
                                              const RCP<const Comm<int> > &comm)
{
  pamgen_adapter_t * ia = nullptr; // pointer for basic vector adapter
  if(uinput->hasPamgenMesh())
  {

    if(uinput->hasPamgenMesh())
    {
      if(comm->getRank() == 0) cout << "Have pamgen mesh, make adapter...." << endl;
      ia = new pamgen_adapter_t(*(comm.get()), "region");
      if(comm->getRank() == 0)
        ia->print(0);
    }
  }else{
    throw std::runtime_error("Pamgen mesh is not available for PamgenMeshAdapter!");
  }
  
  return  reinterpret_cast<AdapterForTests::base_adapter_t *>(ia);
}
#endif


