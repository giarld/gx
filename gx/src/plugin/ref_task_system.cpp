//
// Created by Gxin on 2022/6/5.
//

#include <gx/gany.h>

#include "gx/gtasksystem.h"

#include "gx/debug.h"


using namespace gany;

void refTaskSystem()
{
    Class<GTaskSystem>("Gx", "GTaskSystem",
                      "Gx task system, A multithreaded task system that supports synchronous waiting for task results.")
            .inherit<GObject>()
            .construct<>({"Default constructor, Number of threads created according to the number of CPU cores."})
            .construct<std::string, int32_t>({"Constructor.", {"threadName", "threadCount"}})
            .func("threadCount", &GTaskSystem::threadCount, {"Get number of worker threads."})
            .func("start", &GTaskSystem::start, {"Start the TaskSystem after calling this function."})
            .func("stopAndWait", &GTaskSystem::stopAndWait,
                  {"Stop the TaskSystem after all tasks in the task queue are completed."})
            .func("stop", &GTaskSystem::stop,
                  {"Clear the task queue and stop the TaskSystem. Unexecuted tasks will not be executed."})
            .func("isRunning", &GTaskSystem::isRunning, {"Check whether the TaskSystem is running."})
            .func("setThreadPriority", &GTaskSystem::setThreadPriority, {"Set thread priority.", {"priority"}})
            .func("getThreadPriority", &GTaskSystem::getThreadPriority, {"Get thread priority."})
            .func("submit",
                  GAnyFunction::createVariadicFunction(
                      "",
                      [](const GAny **args, int32_t argc) -> GAny {
                          if (argc < 2) {
                              return GAnyException("Unknown method overload");
                          }
                          if (!args[0]->is<GTaskSystem>()) {
                              return GAnyException("Arg self exception");
                          }
                          if (!args[1]->isFunction()) {
                              return GAnyException("Arg1 must be a function");
                          }
                          auto &self = const_cast<GTaskSystem &>(*args[0]->as<GTaskSystem>());
                          GAny runnable = *args[1];

                          std::vector<GAny> params;
                          for (size_t i = 2; i < argc; i++) {
                              params.push_back(*args[i]);
                          }

                          return GAny::New<GTaskSystem::Task<GAny> >(
                              std::move(self.submit([runnable, params]() {
                                  auto r = runnable._call(params);
                                  CHECK_CONDITION_S_R(!r.isException(), r, "TaskSystem runnable error: {}.", r.as<GAnyException>()->what());
                                  return r;
                              })));
                      }),
                  {
                      "Submit a task to the task queue.",
                      {"runnable:function", "..."},
                      "Task"
                  })
            .func("submitFront",
                  GAnyFunction::createVariadicFunction(
                      "",
                      [](const GAny **args, int32_t argc) -> GAny {
                          if (argc < 2) {
                              return GAnyException("Unknown method overload");
                          }
                          if (!args[0]->is<GTaskSystem>()) {
                              return GAnyException("Arg self exception");
                          }
                          if (!args[1]->isFunction()) {
                              return GAnyException("Arg1 must be a function");
                          }
                          auto &self = const_cast<GTaskSystem &>(*args[0]->as<GTaskSystem>());
                          GAny runnable = *args[1];

                          std::vector<GAny> params;
                          for (size_t i = 2; i < argc; i++) {
                              params.push_back(*args[i]);
                          }

                          return GAny::New<GTaskSystem::Task<GAny> >(
                              std::move(self.submitFront([runnable, params]() {
                                  auto r = runnable._call(params);
                                  CHECK_CONDITION_S_R(!r.isException(), r, "TaskSystem runnable error: {}.", r.as<GAnyException>()->what());
                                  return r;
                              })));
                      }),
                  {
                      "Submit a task to the task queue header.",
                      {"runnable:function", "..."},
                      "Task"
                  })
            .func("waitingTaskCount", &GTaskSystem::waitingTaskCount,
                  {"Get the count of tasks waiting."});

    Class<GTaskSystem::Task<GAny> >("Gx", "Task", "Task results of TaskSystem.")
            .func("get",
                  [](GTaskSystem::Task<GAny> &self) {
                      return self.get();
                  }, {"Wait for the task result, and wait until the task is completed and returned."})
            .func("wait",
                  [](GTaskSystem::Task<GAny> &self) {
                      self.wait();
                  }, {"Block waiting for task to complete."})
            .func("waitFor",
                  [](GTaskSystem::Task<GAny> &self, int64_t ms) {
                      return self.waitFor(ms);
                  },
                  {
                      "Blocking and waiting for task completion within a certain time, "
                      "if the task is completed within the specified time, true will be returned; otherwise, "
                      "false will be returned.",
                      {"ms"}
                  })
            .func("isCompleted",
                  [](GTaskSystem::Task<GAny> &self) {
                      self.isCompleted();
                  },
                  {"Check if the task is completed, equivalent to Task.waitFor(0)."})
            .func("cancel",
                  [](GTaskSystem::Task<GAny> &self) {
                      self.cancel();
                  }, {"Cancel task."})
            .func("isValid",
                  [](GTaskSystem::Task<GAny> &self) {
                      return self.isValid();
                  }, {"Whether the task is valid. If it is canceled or got, the task will be invalid."});
}
