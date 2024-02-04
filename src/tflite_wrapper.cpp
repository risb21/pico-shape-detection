#include "tflite_wrapper.hpp"

bool TFLMicro::is_successful(TfLiteStatus s) {
    if (s != kTfLiteOk)
        return false;
    
    return true;
} 

TFLMicro::TFLMicro(const unsigned char tflite_model[], int tensor_arena_size) :
    _tflite_model(tflite_model),
    _tensor_arena_size(tensor_arena_size),
    _tensor_arena(nullptr),
    _model(nullptr),
    _interpreter(nullptr),
    _input_tensor(nullptr),
    _output_tensor(nullptr)
{
}

TFLMicro::~TFLMicro() {
    if (_interpreter != nullptr) {
        delete _interpreter;
        _interpreter = nullptr;
    }

    if (_model != nullptr) {
        delete _model;
        _model = nullptr;
    }

    if (_input_tensor != nullptr) {
        delete _input_tensor;
        _input_tensor = nullptr;
    }

    if (_output_tensor != nullptr) {
        delete _output_tensor;
        _output_tensor = nullptr;
    }

    if (_tensor_arena != nullptr) {
        delete [] _tensor_arena;
        _tensor_arena = nullptr;
    }
}

int TFLMicro::init() {
    _model = tflite::GetModel(_tflite_model);
    if (_model -> version() != TFLITE_SCHEMA_VERSION) {
        MicroPrintf("Model Provided is schema verison: %d,\n"
                    "not equal to supported verison: %d,\n",
                    _model -> version(), TFLITE_SCHEMA_VERSION);
        return 0;
    }

    _tensor_arena = new uint8_t[_tensor_arena_size];
    if (_tensor_arena == nullptr) {
        MicroPrintf("Failed to allocate tensor arena\n");
        return 0;
    }

    static tflite::MicroMutableOpResolver<4> resolver;
    if (!is_successful(resolver.AddFullyConnected()))
        goto op_error;
    if (!is_successful(resolver.AddReshape()))
        goto op_error;
    if (!is_successful(resolver.AddSoftmax()))
        goto op_error;
    if (!is_successful(resolver.AddRelu()))
        goto op_error;

    static tflite::MicroInterpreter static_interpreter(
        _model, resolver, _tensor_arena, _tensor_arena_size);
    _interpreter = &static_interpreter;

    if (!is_successful(_interpreter -> AllocateTensors())) {
        MicroPrintf("AllocateTensors() failed");
        return 0;
    }

    _input_tensor = static_interpreter.input(0);
    _output_tensor = static_interpreter.output(0);

    return 1;

    op_error:
        MicroPrintf("Op resolution failed");
        return 0;
}

void TFLMicro::input_data(acc_3D<float> *data_arr, size_t size) {
    if (_input_tensor == nullptr) {
        MicroPrintf("The input tensor does not exist\n");
        return;
    }

    float *input_ptr = reinterpret_cast<float *>(_input_tensor -> data.data);

    for (size_t i = 0; i < size; i++) {
        input_ptr[i*3 + 0] = data_arr[i].x;
        input_ptr[i*3 + 1] = data_arr[i].y;
        input_ptr[i*3 + 2] = data_arr[i].z;
    }
}

void* TFLMicro::predict() {
    TfLiteStatus invoke_status = _interpreter -> Invoke();

    if (invoke_status != kTfLiteOk) {
        MicroPrintf("Could not invoke interpreter\n");
        return nullptr;
    }

    return _output_tensor -> data.data;
}