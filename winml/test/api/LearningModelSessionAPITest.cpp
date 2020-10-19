// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "testPch.h"

#include "APITest.h"
#include "CommonDeviceHelpers.h"
#include "LearningModelSessionAPITest.h"
#include "protobufHelpers.h"
#include "winrt/Windows.Storage.h"

#include "winrt/Microsoft.AI.MachineLearning.Experimental.h"

#include <D3d11_4.h>
#include <dxgi1_6.h>
#include "Psapi.h"

using namespace winrt;
using namespace winml;
using namespace wfc;

using wf::IPropertyValue;

static void LearningModelSessionAPITestsClassSetup() {
  init_apartment();
}

static void CreateSessionDeviceDefault()
{
    LearningModel learningModel = nullptr;
    LearningModelDevice learningModelDevice = nullptr;
    WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));

  WINML_EXPECT_NO_THROW(learningModelDevice = LearningModelDevice(LearningModelDeviceKind::Default));
  WINML_EXPECT_NO_THROW(LearningModelSession(learningModel, learningModelDevice));
}

static void CreateSessionDeviceCpu() {
  LearningModel learningModel = nullptr;
  LearningModelDevice learningModelDevice = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));

  WINML_EXPECT_NO_THROW(learningModelDevice = LearningModelDevice(LearningModelDeviceKind::Cpu));
  WINML_EXPECT_NO_THROW(LearningModelSession(learningModel, learningModelDevice));
  // for the CPU device, make sure that we get back NULL and 0 for any device properties
  WINML_EXPECT_EQUAL(learningModelDevice.Direct3D11Device(), nullptr);
  LARGE_INTEGER id;
  id.QuadPart = APITest::GetAdapterIdQuadPart(learningModelDevice);
  WINML_EXPECT_EQUAL(id.LowPart, static_cast<DWORD>(0));
  WINML_EXPECT_EQUAL(id.HighPart, 0);
}

static void CreateSessionWithModelLoadedFromStream()
{
  LearningModel learningModel = nullptr;
  LearningModelDevice learningModelDevice = nullptr;
  std::wstring path = FileHelpers::GetModulePath() + L"model.onnx";
  auto storageFile = ws::StorageFile::GetFileFromPathAsync(path).get();

  WINML_EXPECT_NO_THROW(learningModel = LearningModel::LoadFromStream(storageFile));

  WINML_EXPECT_NO_THROW(learningModelDevice = LearningModelDevice(LearningModelDeviceKind::Default));
  WINML_EXPECT_NO_THROW(LearningModelSession(learningModel, learningModelDevice));
}

static void CreateSessionDeviceDirectX() {
  LearningModel learningModel = nullptr;
  LearningModelDevice learningModelDevice = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));

  WINML_EXPECT_NO_THROW(learningModelDevice = LearningModelDevice(LearningModelDeviceKind::DirectX));
  WINML_EXPECT_NO_THROW(LearningModelSession(learningModel, learningModelDevice));
}

static void CreateSessionDeviceDirectXHighPerformance() {
  LearningModel learningModel = nullptr;
  LearningModelDevice learningModelDevice = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));

  WINML_EXPECT_NO_THROW(learningModelDevice = LearningModelDevice(LearningModelDeviceKind::DirectXHighPerformance));
  WINML_EXPECT_NO_THROW(LearningModelSession(learningModel, learningModelDevice));
}

static void CreateSessionDeviceDirectXMinimumPower() {
  LearningModel learningModel = nullptr;
  LearningModelDevice learningModelDevice = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));

  WINML_EXPECT_NO_THROW(learningModelDevice = LearningModelDevice(LearningModelDeviceKind::DirectXMinPower));
  WINML_EXPECT_NO_THROW(LearningModelSession(learningModel, learningModelDevice));
}

static void AdapterIdAndDevice() {
  LearningModel learningModel = nullptr;
  LearningModelDevice learningModelDevice = nullptr;
  LearningModelSession learningModelSession = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));

  com_ptr<IDXGIFactory6> factory;
  WINML_EXPECT_HRESULT_SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory6), factory.put_void()));
  com_ptr<IDXGIAdapter> adapter;

  learningModelDevice = LearningModelDevice(LearningModelDeviceKind::DirectX);
  WINML_EXPECT_HRESULT_SUCCEEDED(factory->EnumAdapters(0, adapter.put()));
  DXGI_ADAPTER_DESC desc;
  WINML_EXPECT_HRESULT_SUCCEEDED(adapter->GetDesc(&desc));
  LARGE_INTEGER id;
  id.QuadPart = APITest::GetAdapterIdQuadPart(learningModelDevice);
  WINML_EXPECT_EQUAL(desc.AdapterLuid.LowPart, id.LowPart);
  WINML_EXPECT_EQUAL(desc.AdapterLuid.HighPart, id.HighPart);
  WINML_EXPECT_TRUE(learningModelDevice.Direct3D11Device() != nullptr);

  learningModelDevice = LearningModelDevice(LearningModelDeviceKind::DirectXHighPerformance);
  adapter = nullptr;
  WINML_EXPECT_HRESULT_SUCCEEDED(factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter), adapter.put_void()));
  WINML_EXPECT_HRESULT_SUCCEEDED(adapter->GetDesc(&desc));
  id.QuadPart = APITest::GetAdapterIdQuadPart(learningModelDevice);
  WINML_EXPECT_EQUAL(desc.AdapterLuid.LowPart, id.LowPart);
  WINML_EXPECT_EQUAL(desc.AdapterLuid.HighPart, id.HighPart);
  WINML_EXPECT_TRUE(learningModelDevice.Direct3D11Device() != nullptr);

  adapter = nullptr;
  learningModelDevice = LearningModelDevice(LearningModelDeviceKind::DirectXMinPower);
  WINML_EXPECT_HRESULT_SUCCEEDED(factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_MINIMUM_POWER, __uuidof(IDXGIAdapter), adapter.put_void()));
  WINML_EXPECT_HRESULT_SUCCEEDED(adapter->GetDesc(&desc));
  id.QuadPart = APITest::GetAdapterIdQuadPart(learningModelDevice);
  WINML_EXPECT_EQUAL(desc.AdapterLuid.LowPart, id.LowPart);
  WINML_EXPECT_EQUAL(desc.AdapterLuid.HighPart, id.HighPart);
  WINML_EXPECT_TRUE(learningModelDevice.Direct3D11Device() != nullptr);

  WINML_EXPECT_NO_THROW(learningModelSession = LearningModelSession(learningModel, learningModelDevice));
  WINML_EXPECT_EQUAL(learningModelSession.Device().AdapterId(), learningModelDevice.AdapterId());
}

static void EvaluateFeatures() {
  std::vector<int64_t> shape = {4};
  std::vector<winrt::hstring> data = {L"one", L"two", L"three", L"four"};

  // create from buffer
  auto tensor = TensorString::CreateFromArray(shape, data);
  WINML_EXPECT_EQUAL(tensor.GetAsVectorView().Size(), data.size());
  WINML_EXPECT_TRUE(std::equal(data.cbegin(), data.cend(), begin(tensor.GetAsVectorView())));

  // create from vector view
  auto dataCopy = data;
  tensor = TensorString::CreateFromIterable(
      shape, winrt::single_threaded_vector<winrt::hstring>(std::move(dataCopy)).GetView());
  WINML_EXPECT_EQUAL(tensor.GetAsVectorView().Size(), data.size());
  WINML_EXPECT_TRUE(std::equal(data.cbegin(), data.cend(), begin(tensor.GetAsVectorView())));

  LearningModel learningModel = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"id-tensor-string.onnx", learningModel));
  LearningModelSession session(learningModel);

  auto outputTensor = TensorString::Create();

  std::map<hstring, wf::IInspectable> featuresstandardmap;
  featuresstandardmap[L"X"] = tensor;
  featuresstandardmap[L"Y"] = outputTensor;
  auto featureswinrtmap = winrt::single_threaded_map(std::move(featuresstandardmap));
  session.EvaluateFeatures(featureswinrtmap, L"0");

  // verify identity model round-trip works
  WINML_EXPECT_EQUAL(outputTensor.GetAsVectorView().Size(), data.size());
  WINML_EXPECT_TRUE(std::equal(data.cbegin(), data.cend(), begin(outputTensor.GetAsVectorView())));
}

static void EvaluateFeaturesAsync() {
  std::vector<int64_t> shape = {4};
  std::vector<winrt::hstring> data = {L"one", L"two", L"three", L"four"};

  // create from buffer
  auto tensor = TensorString::CreateFromArray(shape, data);
  WINML_EXPECT_EQUAL(tensor.GetAsVectorView().Size(), data.size());
  WINML_EXPECT_TRUE(std::equal(data.cbegin(), data.cend(), begin(tensor.GetAsVectorView())));

  // create from vector view
  auto dataCopy = data;
  tensor = TensorString::CreateFromIterable(
      shape, winrt::single_threaded_vector<winrt::hstring>(std::move(dataCopy)).GetView());
  WINML_EXPECT_EQUAL(tensor.GetAsVectorView().Size(), data.size());
  WINML_EXPECT_TRUE(std::equal(data.cbegin(), data.cend(), begin(tensor.GetAsVectorView())));

  LearningModel learningModel = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"id-tensor-string.onnx", learningModel));
  LearningModelSession session(learningModel);

  auto outputTensor = TensorString::Create(shape);

  std::map<hstring, wf::IInspectable> featuresstandardmap;
  featuresstandardmap[L"X"] = tensor;
  featuresstandardmap[L"Y"] = outputTensor;
  auto featureswinrtmap = winrt::single_threaded_map(std::move(featuresstandardmap));
  session.EvaluateFeaturesAsync(featureswinrtmap, L"0").get();

  // verify identity model round-trip works
  WINML_EXPECT_EQUAL(outputTensor.GetAsVectorView().Size(), data.size());
  WINML_EXPECT_TRUE(std::equal(data.cbegin(), data.cend(), begin(outputTensor.GetAsVectorView())));
}

static void EvaluationProperties() {
  // load a model
  LearningModel learningModel = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));
  // create a session
  LearningModelSession learningModelSession = nullptr;
  learningModelSession = LearningModelSession(learningModel);
  // set a property
  auto value = winrt::Windows::Foundation::PropertyValue::CreateBoolean(true);
  learningModelSession.EvaluationProperties().Insert(L"propName1", value);
  // get the property and make sure it's there with the right value
  auto value2 = learningModelSession.EvaluationProperties().Lookup(L"propName1");
  WINML_EXPECT_EQUAL(value2.as<IPropertyValue>().GetBoolean(), true);
}

static LearningModelSession CreateSession(LearningModel model) {
  LearningModelDevice device(nullptr);
  WINML_EXPECT_NO_THROW(device = LearningModelDevice(LearningModelDeviceKind::DirectX));

  LearningModelSession session(nullptr);
  if (CommonDeviceHelpers::IsFloat16Supported(device)) {
    WINML_EXPECT_NO_THROW(session = LearningModelSession(model, device));
  } else {
    WINML_EXPECT_THROW_SPECIFIC(
        session = LearningModelSession(model, device),
        winrt::hresult_error,
        [](const winrt::hresult_error& e) -> bool {
          return e.code() == DXGI_ERROR_UNSUPPORTED;
        });
  }

  return session;
}

static void CreateSessionWithCastToFloat16InModel() {
  // load a model
  LearningModel learningModel = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"fp16-truncate-with-cast.onnx", learningModel));

  CreateSession(learningModel);
}

static void CreateSessionWithFloat16InitializersInModel()
{
    // load a model
    LearningModel learningModel = nullptr;
    WINML_EXPECT_NO_THROW(APITest::LoadModel(L"fp16-initializer.onnx", learningModel));

  CreateSession(learningModel);
}

static void EvaluateSessionAndCloseModelHelper(
    LearningModelDeviceKind kind,
    bool close_model_on_session_creation) {
  auto shape = std::vector<int64_t>{1, 1000};

  auto model = ProtobufHelpers::CreateModel(TensorKind::Float, shape, 1000);

  auto device = LearningModelDevice(kind);
  auto options = LearningModelSessionOptions();

  // close the model on session creation
  options.CloseModelOnSessionCreation(close_model_on_session_creation);

  // ensure you can create a session from the model
  LearningModelSession session(nullptr);

  WINML_EXPECT_NO_THROW(session = LearningModelSession(model, device, options));

  std::vector<float> input(1000);
  std::iota(std::begin(input), std::end(input), 0.0f);
  auto tensor_input = TensorFloat::CreateFromShapeArrayAndDataArray(shape, input);
  auto binding = LearningModelBinding(session);
  binding.Bind(L"input", tensor_input);

  LearningModelEvaluationResult result(nullptr);
  WINML_EXPECT_NO_THROW(result = session.Evaluate(binding, L""));

  if (close_model_on_session_creation) {
    // ensure that the model has been closed
    WINML_EXPECT_THROW_SPECIFIC(
        LearningModelSession(model, device, options),
        winrt::hresult_error,
        [](const winrt::hresult_error& e) -> bool {
          return e.code() == E_INVALIDARG;
        });
  } else {
    WINML_EXPECT_NO_THROW(LearningModelSession(model, device, options));
  }
}

static void EvaluateSessionAndCloseModel() {
  WINML_EXPECT_NO_THROW(::EvaluateSessionAndCloseModelHelper(LearningModelDeviceKind::Cpu, true));
  WINML_EXPECT_NO_THROW(::EvaluateSessionAndCloseModelHelper(LearningModelDeviceKind::Cpu, false));
}

static void NamedDimensionOverride() 
{
  LearningModel model = nullptr;
  WINML_EXPECT_NO_THROW(APITest::LoadModel(L"fns-candy.onnx", model));

  LearningModelDevice device(nullptr);
  WINML_EXPECT_NO_THROW(device = LearningModelDevice(LearningModelDeviceKind::Cpu));

  // the model input shape. the batch size, n, is overriden to 5
  int64_t n = 5, c = 3, h = 720, w = 720;

  LearningModelSessionOptions options;
  options.OverrideNamedDimension(L"None", n);
  
  // Verifies that if a Dim name doesn't exist the named dimension override does nothing
  options.OverrideNamedDimension(L"DimNameThatDoesntExist", n);

  LearningModelSession session(nullptr);
  WINML_EXPECT_NO_THROW(session = LearningModelSession(model, device, options));

  ILearningModelFeatureDescriptor descriptor = model.InputFeatures().GetAt(0);
  TensorFeatureDescriptor tensorDescriptor = nullptr;
  descriptor.as(tensorDescriptor);
  std::vector<int64_t> shape{n,c,h,w};
  int64_t size = n*c*h*w;
  std::vector<float> buffer;
  buffer.resize(static_cast<size_t>(size));
  auto featureValue = TensorFloat::CreateFromIterable(shape, winrt::single_threaded_vector<float>(std::move(buffer)));
  LearningModelBinding binding(session);
  binding.Bind(descriptor.Name(), featureValue);

  WINML_EXPECT_NO_THROW(session.Evaluate(binding, L""));
}

static void CloseSession()
{
    LearningModel learningModel = nullptr;
    WINML_EXPECT_NO_THROW(APITest::LoadModel(L"model.onnx", learningModel));
    LearningModelSession session = nullptr;

  /*
    HANDLE currentProcessHandle = NULL;
    try
    {
        currentProcessHandle = GetCurrentProcess();
    }
    catch (...)
    {
        VERIFY_FAIL(L"Failed to get current process handle.");
    }
    PROCESS_MEMORY_COUNTERS pmc = { 0 };
    SIZE_T beforeSessionCloseWorkingSetSize = 0;
    SIZE_T afterSessionCloseWorkingSetSize = 0;
    bool getProcessMemoryInfoSuccess = false;
    */
  WINML_EXPECT_NO_THROW(session = LearningModelSession(learningModel));

  /*
    // Get the current process memory info after session creation.
    getProcessMemoryInfoSuccess = GetProcessMemoryInfo(currentProcessHandle, &pmc, sizeof(pmc));
    if (!getProcessMemoryInfoSuccess)
    {
        VERIFY_FAIL(L"Failed to get current process memory info.");
    }
    beforeSessionCloseWorkingSetSize = pmc.WorkingSetSize;
    pmc = { 0 };
    */
  WINML_EXPECT_NO_THROW(session.Close());

  /*
    Bug 23659026: Working set difference tolerance is too tight for LearningModelSessionAPITests::CloseSession
    https://microsoft.visualstudio.com/OS/_workitems/edit/23659026

    // Check that working set size has dropped after session close
    getProcessMemoryInfoSuccess = GetProcessMemoryInfo(currentProcessHandle, &pmc, sizeof(pmc));
    if (!getProcessMemoryInfoSuccess)
    {
        VERIFY_FAIL(L"Failed to get current process memory info.");
    }
    afterSessionCloseWorkingSetSize = pmc.WorkingSetSize;
    pmc = { 0 };

    // expected working set difference of session close. It is approximately 2x the size of the weights of model.onnx
    // there needs to be a tolerance because the working set difference varies from run to run.

    // Bug 23739697: Closing Session API in LearningModelSessionAPITests::CloseSession doesn't always result in ~2x working set memory reduction.
    // https://microsoft.visualstudio.com/OS/_workitems/edit/23739697
    float tolerance = 0.4f;
    int64_t expectedWorkingSetDifference = 9662464;
    VERIFY_IS_LESS_THAN(expectedWorkingSetDifference - (beforeSessionCloseWorkingSetSize - afterSessionCloseWorkingSetSize), expectedWorkingSetDifference * tolerance);
    */

  // verify that model still has metadata info after session close
  std::wstring author(learningModel.Author());
  WINML_EXPECT_EQUAL(author, L"onnx-caffe2");

  // verify that session throws RO_E_CLOSED error
  std::vector<float> input(1 * 3 * 224 * 224, 0);
  std::vector<int64_t> shape = {1, 3, 224, 224};
  auto tensor_input = TensorFloat::CreateFromShapeArrayAndDataArray(shape, input);
  WINML_EXPECT_THROW_SPECIFIC(LearningModelBinding binding(session),
                              winrt::hresult_error,
                              [](const winrt::hresult_error& e) -> bool {
                                return e.code() == RO_E_CLOSED;
                              });
}

static void TestSpectrogram() {
  printf("\nTestSpectrogram\n");
  using namespace winml_experimental;
  using Operator = winml_experimental::LearningModelOperator;

  static const wchar_t ONNX_DOMAIN[] = L"";
  static const wchar_t MS_DOMAIN[] = L"com.microsoft";

  std::vector<int64_t> scalar_shape = {};
  std::vector<int64_t> shape = {1, 32};
  std::vector<int64_t> output_shape = {1, 32, 1};
  std::vector<int64_t> slice_shape = {3};
  std::vector<int64_t> input_slice_shape = {1};
  std::vector<int64_t> mel_shape = { 32, 10 };

  // Constant initializers

  // real slice
  auto real_slice_start = TensorInt32Bit::CreateFromShapeArrayAndDataArray(slice_shape, {0, 0, 0});
  auto real_slice_ends = TensorInt32Bit::CreateFromShapeArrayAndDataArray(slice_shape, {INT_MAX, INT_MAX, 1});

  // complex slice
  auto complex_slice_start = TensorInt32Bit::CreateFromShapeArrayAndDataArray(slice_shape, {0, 0, 1});
  auto complex_slice_ends = TensorInt32Bit::CreateFromShapeArrayAndDataArray(slice_shape, {INT_MAX, INT_MAX, 2});
  auto power_of_2_exponent = TensorFloat::CreateFromShapeArrayAndDataArray(scalar_shape, {2});
  auto number_of_samples = TensorFloat::CreateFromShapeArrayAndDataArray(scalar_shape, {32});

  // input slice
  auto input_slice_start = TensorInt32Bit::CreateFromShapeArrayAndDataArray(input_slice_shape, {1});
  auto input_slice_ends = TensorInt32Bit::CreateFromShapeArrayAndDataArray(input_slice_shape, {2});

  // Mel spectrogram
  auto num_mel_bins = TensorInt64Bit::CreateFromShapeArrayAndDataArray(scalar_shape, {10});
  auto num_spectrogram_bins = TensorInt64Bit::CreateFromShapeArrayAndDataArray(scalar_shape, {32});
  auto sample_rate = TensorInt64Bit::CreateFromShapeArrayAndDataArray(scalar_shape, {48000});
  auto lower_edge_hertz = TensorFloat::CreateFromShapeArrayAndDataArray(scalar_shape, {0});
  auto upper_edge_hertz = TensorFloat::CreateFromShapeArrayAndDataArray(scalar_shape, {8000});

  auto model =
      LearningModelBuilder::Create()
          // Inputs
          .Inputs().Add(TensorFeatureDescriptor(L"Input", L"The input time domain signal", TensorKind::Float, shape))
          .Inputs().Add(TensorFeatureDescriptor(L"NumSamples", L"Number of samples in audio signal", TensorKind::Float, scalar_shape), number_of_samples) // This is only needed becase a cast is necessary and unsupported due to lack of attribute support

          // Outputs
          .Outputs().Add(TensorFeatureDescriptor(L"Output", L"The real output of the frequency domain spectra", TensorKind::Float, output_shape))
          .Outputs().Add(TensorFeatureDescriptor(L"MelMatrixOutput", L"The mel matrix", TensorKind::Float, mel_shape))

          // real slice
          .Inputs().AddConstant(TensorFeatureDescriptor(L"real_slice_start", L"The starts to real slice the spectra", TensorKind::Int32, slice_shape), real_slice_start)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"real_slice_ends", L"The ends to real slice the spectra", TensorKind::Int32, slice_shape), real_slice_ends)

          // complex slice
          .Inputs().AddConstant(TensorFeatureDescriptor(L"complex_slice_start", L"The starts to complex slice the spectra", TensorKind::Int32, slice_shape), complex_slice_start)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"complex_slice_ends", L"The ends to complex slice the spectra", TensorKind::Int32, slice_shape), complex_slice_ends)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"power_of_2_exponent", L"The power of 2", TensorKind::Float, scalar_shape), power_of_2_exponent)

           // input slice
          .Inputs().AddConstant(TensorFeatureDescriptor(L"input_slice_start", L"The starts to the input shape slice", TensorKind::Int32, input_slice_shape), input_slice_start)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"input_slice_ends", L"The ends to the input shape slice", TensorKind::Int32, input_slice_shape), input_slice_ends)

          // Mel spectrogram
          .Inputs().AddConstant(TensorFeatureDescriptor(L"num_mel_bins", L"The number of mel bins", TensorKind::Int64, scalar_shape), num_mel_bins)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"num_spectrogram_bins", L"The number os spectrogram bins", TensorKind::Int64, scalar_shape), num_spectrogram_bins)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"sample_rate", L"The sample rate", TensorKind::Int64, scalar_shape), sample_rate)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"lower_edge_hertz", L"The lower hertz edge of the mel spectrogram", TensorKind::Float, scalar_shape), lower_edge_hertz)
          .Inputs().AddConstant(TensorFeatureDescriptor(L"upper_edge_hertz", L"The upper hertz edge of the mel spectrogram", TensorKind::Float, scalar_shape), upper_edge_hertz)

          // The graph
          .Operators().Add(Operator(L"Shape", L"shape0", ONNX_DOMAIN)
              .SetInput(L"data", L"Input")
              .SetOutput(L"shape", L"input_shape"))
          .Operators().Add(Operator(L"Slice", L"shape_slice", ONNX_DOMAIN)
              .SetInput(L"data", L"input_shape")
              .SetInput(L"starts", L"input_slice_start")
              .SetInput(L"ends", L"input_slice_ends")
              .SetOutput(L"output", L"num_samples"))
          .Operators().Add(Operator(L"HannWindow", L"hann0", MS_DOMAIN)
              .SetInput(L"size", L"num_samples")
              .SetOutput(L"output", L"hann_window"))
          .Operators().Add(Operator(L"Mul", L"mul0", ONNX_DOMAIN)
              .SetInput(L"A", L"hann_window")
              .SetInput(L"B", L"Input")
              .SetOutput(L"C", L"windowed_signal"))
          .Operators().Add(Operator(L"DFT", L"dft0", MS_DOMAIN)
              .SetInput(L"input", L"windowed_signal")
              .SetOutput(L"output", L"spectra"))
          .Operators().Add(Operator(L"Slice", L"real_slice", ONNX_DOMAIN)
              .SetInput(L"data", L"spectra")
              .SetInput(L"starts", L"real_slice_start") // .SetConstant(L"starts", TensorInt32Bit::CreateFromShapeArrayAndDataArray({3}, {0, 0, 0}))
              .SetInput(L"ends", L"real_slice_ends")
              .SetOutput(L"output", L"reals"))
          .Operators().Add(Operator(L"Slice", L"complex_slice", ONNX_DOMAIN)
              .SetInput(L"data", L"spectra")
              .SetInput(L"starts", L"complex_slice_start")
              .SetInput(L"ends", L"complex_slice_ends")
              .SetOutput(L"output", L"imaginaries"))
          .Operators().Add(Operator(L"Pow", L"real_pow", ONNX_DOMAIN)
              .SetInput(L"X", L"reals")
              .SetInput(L"Y", L"power_of_2_exponent")
              .SetOutput(L"Z", L"reals_squared"))
          .Operators().Add(Operator(L"Pow", L"complex_pow", ONNX_DOMAIN)
              .SetInput(L"X", L"imaginaries")
              .SetInput(L"Y", L"power_of_2_exponent")
              .SetOutput(L"Z", L"imaginaries_squared"))
          .Operators().Add(Operator(L"Add", L"add0", ONNX_DOMAIN)
              .SetInput(L"A", L"reals_squared")
              .SetInput(L"B", L"imaginaries_squared")
              .SetOutput(L"C", L"magnitude_squared"))
          .Operators().Add(Operator(L"Sqrt", L"sqrt0", ONNX_DOMAIN)
              .SetInput(L"X", L"magnitude_squared")
              .SetOutput(L"Y", L"magnitude"))
          .Operators().Add(Operator(L"Div", L"div0", ONNX_DOMAIN)
              .SetInput(L"A", L"magnitude")
              .SetInput(L"B", L"NumSamples")
              .SetOutput(L"C", L"Output"))
          .Operators().Add(Operator(L"MelWeightMatrix", L"melweightmatrix0", MS_DOMAIN)
              .SetInput(L"num_mel_bins", L"num_mel_bins")
              .SetInput(L"num_spectrogram_bins", L"num_spectrogram_bins")
              .SetInput(L"sample_rate", L"sample_rate")
              .SetInput(L"lower_edge_hertz", L"lower_edge_hertz")
              .SetInput(L"upper_edge_hertz", L"upper_edge_hertz")
              .SetOutput(L"output", L"MelMatrixOutput"))
          .CreateModel();

  LearningModelSession session(model);
  LearningModelBinding binding(session);

  // Populate binding
  std::vector<float> input = {1, 2, 3, 4, 1, 0, 0, 0, 1, 2, 3, 4, 1, 0, 0, 0, 1, 2, 3, 4, 1, 0, 0, 0, 1, 2, 3, 4, 1, 0, 0, 0};
  binding.Bind(L"Input", TensorFloat::CreateFromShapeArrayAndDataArray(shape, input));

  // These are constant initializers... but since that is not implemented... they need to be duplicated
  binding.Bind(L"real_slice_start", real_slice_start);
  binding.Bind(L"real_slice_ends", real_slice_ends);
  binding.Bind(L"complex_slice_start", complex_slice_start);
  binding.Bind(L"complex_slice_ends", complex_slice_ends);
  binding.Bind(L"power_of_2_exponent", power_of_2_exponent);
  binding.Bind(L"NumSamples", number_of_samples); 
  binding.Bind(L"input_slice_start", input_slice_start);
  binding.Bind(L"input_slice_ends", input_slice_ends);
  binding.Bind(L"num_mel_bins", num_mel_bins);
  binding.Bind(L"num_spectrogram_bins",num_spectrogram_bins);
  binding.Bind(L"sample_rate", sample_rate);
  binding.Bind(L"lower_edge_hertz",lower_edge_hertz);
  binding.Bind(L"upper_edge_hertz",upper_edge_hertz);

  // Evaluate
  auto result = session.Evaluate(binding, L"");

  // Check results
  auto y_tensor = result.Outputs().Lookup(L"Output").as<TensorFloat>();
  auto y_ivv = y_tensor.GetAsVectorView();

  for (int i = 0; i < output_shape[0] * output_shape[1]; i++) {
    printf("%f\n", y_ivv.GetAt(i));
  }

  auto mel_tensor = result.Outputs().Lookup(L"MelMatrixOutput").as<TensorFloat>();
  auto mel_ivv = mel_tensor.GetAsVectorView();
  printf("MelWeightMatrix\n");
  for (int i = 0; i < mel_shape[0]; i++) {
    for (int j = 0; j < mel_shape[1]; j++) {
      printf("%.2f,", mel_ivv.GetAt((i * mel_shape[1]) + j));
    }
    printf("\n");
  }
}

static void TestWindowFunction(const wchar_t* window_operator) {
  printf("\n%ls\n", window_operator);
    using namespace winml_experimental;
    using Operator = winml_experimental::LearningModelOperator;

    std::vector<int64_t> scalar_shape = {};
    std::vector<int64_t> output_shape = {32};
    auto double_data_type = TensorInt32Bit::CreateFromArray(std::vector<int64_t>({1}), std::vector<int32_t>({7}));
    auto model = 
        LearningModelBuilder::Create()
                .Inputs().Add(TensorFeatureDescriptor(L"Input", L"The input time domain signal", TensorKind::Int64, scalar_shape))
                .Outputs().Add(TensorFeatureDescriptor(L"Output", L"The output frequency domain spectra", TensorKind::Float, output_shape))
                .Operators().Add(Operator(window_operator, L"window0", L"com.microsoft").SetInput(L"size", L"Input").SetOutput(L"output", L"Output"))//.SetAttribute(L"output_datatype", double_data_type))
                .CreateModel();

    LearningModelSession session(model);
    LearningModelBinding binding(session);

    std::vector<int64_t> x = { 32 };
    binding.Bind(L"Input", TensorInt64Bit::CreateFromShapeArrayAndDataArray(scalar_shape, x));

    // Evaluate
    auto result = session.Evaluate(binding, L"");

    // Check results
    auto y_tensor = result.Outputs().Lookup(L"Output").as<TensorFloat>();
    auto y_ivv = y_tensor.GetAsVectorView();
    for (int i = 0; i < output_shape[0]; i ++) {
      printf("%f\n", y_ivv.GetAt(i));
    }
}


static void TestDFT() {
  printf("\nDFT\n");
  using namespace winml_experimental;
  using Operator = winml_experimental::LearningModelOperator;

  std::vector<int64_t> shape = {1, 5};
  std::vector<int64_t> output_shape = {1, 5, 2};
  auto model =
      LearningModelBuilder::Create()
          .Inputs().Add(TensorFeatureDescriptor(L"Input", L"The input time domain signal", TensorKind::Float, shape))
          .Outputs().Add(TensorFeatureDescriptor(L"Output", L"The output frequency domain spectra", TensorKind::Float, output_shape))
          .Operators().Add(Operator(L"DFT", L"dft0", L"com.microsoft").SetInput(L"input", L"Input").SetOutput(L"output", L"Output"))
          //.Operators().Add(Operator(L"DFT", L"dft0", L"com.microsoft").SetInput(L"input", L"Input").SetOutput(L"output", L"dft0_output"))
          //.Operators().Add(Operator(L"IDFT", L"idft0", L"com.microsoft").SetInput(L"input", L"dft0_output").SetOutput(L"output", L"Output"))
          .CreateModel();

  LearningModelSession session(model);
  LearningModelBinding binding(session);

  // Populate binding
  //std::vector<float> x = {1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  //std::vector<float> x = {1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0};
  //std::vector<float> x = {1, 2, 3, 4, 1, 0, 0, 0, 1, 2, 3, 4, 1, 0, 0, 0, 1, 2, 3, 4, 1, 0, 0, 0, 1, 2, 3, 4, 1, 0, 0, 0};
  //std::vector<float> x = {1, 0, 2, 0, 3, 0, 4, 0};
  std::vector<float> x = {
      1, 2, 3, 4, 5};
  binding.Bind(L"Input", TensorFloat::CreateFromShapeArrayAndDataArray(shape, x));

  // Evaluate
  auto result = session.Evaluate(binding, L"");

  // Check results
  auto y_tensor = result.Outputs().Lookup(L"Output").as<TensorFloat>();
  auto y_ivv = y_tensor.GetAsVectorView();
  for (int i = 0; i < shape[0] * shape[1] * 2; i += 2) {
    printf("%f + %fj\n", y_ivv.GetAt(i), y_ivv.GetAt(i + 1));
  }
}

static void TestGemm() {
  printf("\nGemm\n");
  using namespace winml_experimental;
  using Operator = winml_experimental::LearningModelOperator;

  // TODO: OPERATOR IDL SHOULD CONTAIN IMPLEMENTED OPERATOR TYPES AND BE GENERATED BY THE BUILD
  // TODO: FREE DIMENSIONS ARE NOT IMPLEMENTED
  // TODO: ATTRIBUTES ARE NOT IMPLEMENTED, IE - operator.SetAttribute(L"att", attribute))
  // TODO: CONSTANT INPUTS ARE NOT IMPLEMENTED, IE - builder.Inputs().AddConstant(TensorFeatureDescriptor(L"c", L"the c input", TensorKind::Float, shape), constant_y)
  // TODO: SHOULD LEARNINGMODELBUILDER::CREATE ACCEPT OPSET VERSION MAP?
  // TODO: MAPFEATUREDESCRIPTOR AND SEQUENCEFEATUREDESCRIPTOR CONSTRUCTORS AND ABI METHODS ARE NOT IMPLEMENTED
  //
  // GEMM C INPUT SHOULD BE OPTIONAL BUT IT REQUIRED BY ONNX AS THE IMPLEMENTATION IS NO UP TO DATE

  std::vector<int64_t> shape = {3, 3};
  std::vector<float> x =
  {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
  };
  auto model =
    LearningModelBuilder::Create()
      .Inputs().Add(TensorFeatureDescriptor(L"a", L"the a input", TensorKind::Float, shape))
      .Inputs().Add(TensorFeatureDescriptor(L"b", L"the b input", TensorKind::Float, shape))
      .Inputs().Add(TensorFeatureDescriptor(L"c", L"the c input", TensorKind::Float, shape))
      .Outputs().Add(TensorFeatureDescriptor(L"y", L"the y output", TensorKind::Float, shape))
      .Operators().Add(Operator(L"Gemm", L"gemm0", L"").SetInput(L"A", L"a").SetInput(L"B", L"b").SetInput(L"C", L"c").SetOutput(L"Y", L"y"))
      .CreateModel();

}

static void TestModelBuilding() {
  TestSpectrogram();
  TestWindowFunction(L"HannWindow");
  TestWindowFunction(L"HammingWindow");
  TestWindowFunction(L"BlackmanWindow");
  TestDFT();
  TestGemm();
}

static void SetIntraOpNumThreads() {
    auto shape = std::vector<int64_t>{1, 1000};
    auto model = ProtobufHelpers::CreateModel(TensorKind::Float, shape, 1000);
    auto device = LearningModelDevice(LearningModelDeviceKind::Cpu);
    auto options = LearningModelSessionOptions();
    auto nativeOptions = options.as<ILearningModelSessionOptionsNative>();

    // Set the number of intra op threads to half of logical cores.
    uint32_t desiredThreads = std::thread::hardware_concurrency() / 2;
    WINML_EXPECT_NO_THROW(nativeOptions->SetIntraOpNumThreadsOverride(desiredThreads));
    // Create session and grab the number of intra op threads to see if is set properly
    LearningModelSession session = nullptr;
    WINML_EXPECT_NO_THROW(session = LearningModelSession(model, device, options));
    auto nativeSession = session.as<ILearningModelSessionNative>();
    uint32_t numIntraOpThreads;
    WINML_EXPECT_NO_THROW(nativeSession->GetIntraOpNumThreads(&numIntraOpThreads));
    WINML_EXPECT_EQUAL(desiredThreads, numIntraOpThreads);

    // Check to see that bind and evaluate continue to work when setting the intra op thread count
    std::vector<float> input(1000);
    std::iota(std::begin(input), std::end(input), 0.0f);
    auto tensor_input = TensorFloat::CreateFromShapeArrayAndDataArray(shape, input);
    auto binding = LearningModelBinding(session);
    binding.Bind(L"input", tensor_input);
    WINML_EXPECT_NO_THROW(session.Evaluate(binding, L""));

    // Check to verify that the default number of threads in LearningModelSession is equal to the number of logical cores.
    session = LearningModelSession(model, device);
    nativeSession = session.as<ILearningModelSessionNative>();
    WINML_EXPECT_NO_THROW(nativeSession->GetIntraOpNumThreads(&numIntraOpThreads));
    WINML_EXPECT_EQUAL(std::thread::hardware_concurrency(), numIntraOpThreads);
 }

const LearningModelSessionAPITestsApi& getapi() {
  static LearningModelSessionAPITestsApi api =
  {
    LearningModelSessionAPITestsClassSetup,
    CreateSessionDeviceDefault,
    CreateSessionDeviceCpu,
    CreateSessionWithModelLoadedFromStream,
    CreateSessionDeviceDirectX,
    CreateSessionDeviceDirectXHighPerformance,
    CreateSessionDeviceDirectXMinimumPower,
    AdapterIdAndDevice,
    EvaluateFeatures,
    EvaluateFeaturesAsync,
    EvaluationProperties,
    CreateSessionWithCastToFloat16InModel,
    CreateSessionWithFloat16InitializersInModel,
    EvaluateSessionAndCloseModel,
    NamedDimensionOverride,
    CloseSession,
    SetIntraOpNumThreads,
    TestModelBuilding
  };

  if (SkipGpuTests()) {
    api.CreateSessionDeviceDirectX = SkipTest;
    api.CreateSessionDeviceDirectXHighPerformance = SkipTest;
    api.CreateSessionDeviceDirectXMinimumPower = SkipTest;
    api.CreateSessionWithCastToFloat16InModel = SkipTest;
    api.CreateSessionWithFloat16InitializersInModel = SkipTest;
    api.AdapterIdAndDevice = SkipTest;
  }
  if (RuntimeParameterExists(L"EdgeCore")) {
    api.AdapterIdAndDevice = SkipTest;
  }
  if (RuntimeParameterExists(L"noIDXGIFactory6Tests")) {
    api.CreateSessionDeviceDirectXHighPerformance = SkipTest;
    api.CreateSessionDeviceDirectXMinimumPower = SkipTest;
    api.AdapterIdAndDevice = SkipTest;
  }
  if (SkipTestsImpactedByOpenMP()) {
      api.SetIntraOpNumThreads = SkipTest;
  }
 return api;
}
