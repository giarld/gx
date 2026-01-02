//
// Created by Gxin on 2023/6/17.
//

#include <gx/gany.h>

#include <gx/gjobsystem.h>


using namespace gany;

void refJobSystem()
{
    Class<GJobSystem::Job>("Gx", "Job", "Gx job of job system .")
            .construct<>();

    Class<GJobSystem>("Gx", "GJobSystem", "Gx job system, Used to handle parallel tasks.")
            .inherit<GObject>()
            .construct<>()
            .construct<const std::string &>()
            .construct<const std::string &, uint32_t>()
            .construct<const std::string &, uint32_t, uint32_t>()
            .func("adopt", &GJobSystem::adopt)
            .func("emancipate", &GJobSystem::emancipate)
            .func("setRootJob", &GJobSystem::setRootJob)
            .func("createJob", [](GJobSystem &self, GJobSystem::Job *parent, GJobSystem::JobFunc jobFunc) -> GJobSystem::Job *{
                return self.createJob(parent, jobFunc);
            })
            .func("createJob", [](GJobSystem &self) {
                return self.createJob();
            })
            .func("cancel", &GJobSystem::cancel)
            .func("retain", &GJobSystem::retain)
            .func("release", [](GJobSystem &self, GJobSystem::Job *job) {
                self.release(job);
            })
            .func("run", [](GJobSystem &self, GJobSystem::Job *job) {
                self.run(job);
            })
            .func("signal", &GJobSystem::signal)
            .func("runAndRetain", [](GJobSystem &self, GJobSystem::Job *job) {
                return self.runAndRetain(job);
            })
            .func("waitAndRelease", &GJobSystem::waitAndRelease)
            .func("runAndWait", [](GJobSystem &self, GJobSystem::Job *job) {
                self.runAndWait(job);
            });
}
