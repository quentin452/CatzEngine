#include "LightUtilities.hpp"
class LightProcessor {
  private:
    std::vector<std::thread> threadPool;
    std::mutex mutex;

    void processLight(Light &light) {
        switch (light.type) {
        case LIGHT_DIR:
            processDirectionalLight(light);
            break;
        case LIGHT_POINT:
            processPointLight(light);
            break;
        case LIGHT_LINEAR:
            processLinearLight(light);
            break;
        case LIGHT_CONE:
            processConeLight(light);
            break;
        }
    }

    void processLightForward(Light &light, ALPHA_MODE alpha) {
        switch (light.type) {
        case LIGHT_DIR:
            processDirectionalLightForward(light, alpha);
            break;
        case LIGHT_POINT:
            processPointLightForward(light, alpha);
            break;
        case LIGHT_LINEAR:
            processLinearLightForward(light, alpha);
            break;
        case LIGHT_CONE:
            processConeLightForward(light, alpha);
            break;
        }
    }

  public:
    void drawLights(std::vector<Light> lights) {
        for (auto &light : lights) {
            threadPool.emplace_back(&LightProcessor::processLight, this, light);
        }

        // Join all threads
        for (auto &thread : threadPool) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threadPool.clear();
    }

    void drawLightsForward(std::vector<Light> lights, ALPHA_MODE alpha) {
        for (auto &light : lights) {
            threadPool.emplace_back(&LightProcessor::processLightForward, this, light, alpha);
        }

        // Join all threads
        for (auto &thread : threadPool) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threadPool.clear();
    }
};
/*
class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};

// Usage in LightProcessor
class LightProcessor {
private:
    ThreadPool threadPool{std::thread::hardware_concurrency()}; // Create a thread pool with number of threads equal to hardware concurrency

    void processLight(Light &light) {
        switch (light.type) {
        case LIGHT_DIR:
            processDirectionalLight(light);
            break;
        case LIGHT_POINT:
            processPointLight(light);
            break;
        case LIGHT_LINEAR:
            processLinearLight(light);
            break;
        case LIGHT_CONE:
            processConeLight(light);
            break;
        }
    }

    void processLightForward(Light &light, ALPHA_MODE alpha) {
        switch (light.type) {
        case LIGHT_DIR:
            processDirectionalLightForward(light, alpha);
            break;
        case LIGHT_POINT:
            processPointLightForward(light, alpha);
            break;
        case LIGHT_LINEAR:
            processLinearLightForward(light, alpha);
            break;
        case LIGHT_CONE:
            processConeLightForward(light, alpha);
            break;
        }
    }

public:
    void drawLights(std::vector<Light> lights) {
        for (auto &light : lights) {
            threadPool.enqueue([this, &light] { this->processLight(light); });
        }
        // No need to join threads as they are managed by ThreadPool
    }

    void drawLightsForward(std::vector<Light> lights, ALPHA_MODE alpha) {
        for (auto &light : lights) {
            threadPool.enqueue([this, &light, alpha] { this->processLightForward(light, alpha); });
        }
        // No need to join threads as they are managed by ThreadPool
    }
};*/