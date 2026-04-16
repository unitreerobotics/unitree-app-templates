#pragma once

#include <MNN/Interpreter.hpp>
#include <filesystem>
#include <iostream>
#include <mutex>

namespace isaaclab
{

class Algorithms
{
public:
    virtual ~Algorithms() = default;
    virtual const std::vector<float>& act(const std::unordered_map<std::string, std::vector<float>>& obs) = 0;

    std::vector<float> get_action()
    {
        std::lock_guard<std::mutex> lock(act_mtx_);
        return action;
    }
    
    std::vector<float> action;
protected:
    std::mutex act_mtx_;
};

class MNNRunner : public Algorithms
{
public:
    explicit MNNRunner(const std::filesystem::path& model_path)
    {
        if (!std::filesystem::exists(model_path)) {
            throw std::runtime_error("MNN model not found: " + model_path.string());
        }

        interpreter_.reset(MNN::Interpreter::createFromFile(model_path.c_str()));
        if (!interpreter_) {
            throw std::runtime_error("Failed to load MNN model: " + model_path.string());
        }

        MNN::ScheduleConfig config;
        config.type = MNN_FORWARD_CPU;
        config.numThread = 1;
        config.backupType = MNN_FORWARD_CPU;  

        session_ = interpreter_->createSession(config);
        if (!session_) {
            throw std::runtime_error("Failed to create MNN session for model: " + model_path.string());
        }

        const auto& session_inputs = interpreter_->getSessionInputAll(session_);
        if (session_inputs.empty()) {
            throw std::runtime_error("MNN model has no inputs: " + model_path.string());
        }

        for (const auto& [name, tensor] : session_inputs) {
            input_names_.push_back(name);
            std::vector<int> dims = tensor->shape();
            input_shapes_.push_back(dims);
            input_tensors_.push_back(tensor);

            size_t size = 1;
            for (const auto dim : dims) {
                if (dim <= 0) {
                    throw std::runtime_error("Unsupported dynamic MNN input shape for '" + name + "'.");
                }
                size *= static_cast<size_t>(dim);
            }
            input_sizes_.push_back(size);


            input_host_tensors_.emplace_back(
                std::shared_ptr<MNN::Tensor>(
                    new MNN::Tensor(tensor, tensor->getDimensionType())
                )
            );
        }

        const auto& session_outputs = interpreter_->getSessionOutputAll(session_);
        if (session_outputs.empty()) {
            throw std::runtime_error("MNN model has no outputs: " + model_path.string());
        }

        output_name_ = session_outputs.begin()->first;
        output_tensor_ = session_outputs.begin()->second;


        output_host_tensor_ = std::shared_ptr<MNN::Tensor>(
            new MNN::Tensor(output_tensor_, output_tensor_->getDimensionType())
        );

        action.resize(static_cast<size_t>(output_tensor_->elementSize()));
    }

    const std::vector<float>& act(const std::unordered_map<std::string, std::vector<float>>& obs) override
    {
        const bool single_input = input_names_.size() == 1;
        for (size_t i = 0; i < input_names_.size(); ++i) {
            const auto& input_name = input_names_[i];
            auto obs_it = obs.find(input_name);
            if (obs_it == obs.end() && single_input && obs.size() == 1) {
                obs_it = obs.begin();
            }
            if (obs_it == obs.end()) {
                throw std::runtime_error("Input name '" + input_name + "' not found in observations.");
            }

            auto& input_data = obs_it->second;
            if (input_data.size() != input_sizes_[i]) {
                throw std::runtime_error(
                    "Observation size mismatch for input '" + input_name + "': got " +
                    std::to_string(input_data.size()) + ", expected " + std::to_string(input_sizes_[i]));
            }


            auto& host_tensor = *input_host_tensors_[i];
            std::memcpy(host_tensor.host<float>(), input_data.data(), input_data.size() * sizeof(float));
            input_tensors_[i]->copyFromHostTensor(&host_tensor);
        }

        if (interpreter_->runSession(session_) != MNN::NO_ERROR) {
            throw std::runtime_error("Failed to run MNN session.");
        }

        output_tensor_->copyToHostTensor(output_host_tensor_.get());

        const auto output_size = static_cast<size_t>(output_host_tensor_->elementSize());
        std::lock_guard<std::mutex> lock(act_mtx_);
        std::memcpy(action.data(), output_host_tensor_->host<float>(), output_size * sizeof(float));
        return action;
    }

private:
    struct InterpreterDeleter {
        void operator()(MNN::Interpreter* interpreter) const
        {
            if (interpreter) {
                MNN::Interpreter::destroy(interpreter);
            }
        }
    };

    std::unique_ptr<MNN::Interpreter, InterpreterDeleter> interpreter_;
    MNN::Session* session_ = nullptr;

    std::vector<std::string> input_names_;
    std::vector<std::vector<int>> input_shapes_;
    std::vector<size_t> input_sizes_;
    std::vector<MNN::Tensor*> input_tensors_;
    std::string output_name_;
    MNN::Tensor* output_tensor_ = nullptr;

    std::vector<std::shared_ptr<MNN::Tensor>> input_host_tensors_;
    std::shared_ptr<MNN::Tensor> output_host_tensor_;
};

};
