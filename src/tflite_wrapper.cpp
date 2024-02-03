#include "tflite_wrapper.hpp"

bool TFLMicro::is_op_successful(TfLiteStatus s) {
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
    TfLiteStatus resolve_status = resolver.AddFullyConnected();
    if (!is_op_successful(resolve_status)) {
        MicroPrintf("Op resolution failed");
        return 0;
    }
    resolve_status = resolver.AddReshape();
    if (!is_op_successful(resolve_status)) {
        MicroPrintf("Op resolution failed");
        return 0;
    }
    resolve_status = resolver.AddSoftmax();
    if (!is_op_successful(resolve_status)) {
        MicroPrintf("Op resolution failed");
        return 0;
    }
    resolve_status = resolver.AddRelu();
    if (!is_op_successful(resolve_status)) {
        MicroPrintf("Op resolution failed");
        return 0;
    }

    static tflite::MicroInterpreter static_interpreter(
        _model, resolver, _tensor_arena, _tensor_arena_size);
    _interpreter = &static_interpreter;

    TfLiteStatus allocate_status = _interpreter -> AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        MicroPrintf("AllocateTensors() failed");
        return 0;
    }

    _input_tensor = static_interpreter.input(0);
    // _output_tensor = static_interpreter.output(0);

    return 1;
}

void* TFLMicro::input_data() {
    if (_input_tensor == nullptr) {
        return nullptr;
    }

    return _input_tensor -> data.data;
}

void* TFLMicro::predict() {
    TfLiteStatus invoke_status = _interpreter -> Invoke();

    if (invoke_status != kTfLiteOk) {
        MicroPrintf("Could not Invoke interpreter\n");
        return nullptr;
    }

    _output_tensor = _interpreter -> output(0);

    // float y_quantized = _output_tensor -> data.f;
    // float y = (y_quantized - _output_tensor -> params.zero_point) *
    //           _output_tensor -> params.scale;
    return _output_tensor -> data.data;
}


float TFLMicro::input_scale() const {
    if (_input_tensor == NULL) {
        return NAN;
    }

    return _input_tensor -> params.scale;
}

int32_t TFLMicro::input_zero_point() const {
    if (_input_tensor == NULL) {
        return 0;
    }

    return _input_tensor -> params.zero_point;
}