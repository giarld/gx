//
// Created by Gxin on 2022/5/12.
//

#include <cstdlib>

#include <gx/gtasksystem.h>
#include <gx/gthread.h>

#include <gx/debug.h>


int plus(int a, int b)
{
    Log("Run plus({}, {}), thread: {}", a, b, GThread::currentThreadIdString());
    return a + b;
}

int main(int argc, char *argv[])
{
    GTaskSystem taskSystem;

    taskSystem.start();

    Log("Main thread: {}", GThread::currentThreadIdString());

    {
        taskSystem.submit(
                [] {
                    GThread::sleep(1);
                    Log("Run Task 1, thread: {}", GThread::currentThreadIdString());
                    return 123;
                });
    }

    int a = 10;

    auto task2 = taskSystem.submitFront(plus, std::cref(a), 20);
    auto task3 = taskSystem.submit(
            [](int a, int b) {
                Log("Run Task 3, thread: {}", GThread::currentThreadIdString());
                return a * b;
            }, 20, 30);

    task3.cancel();

    Log("Task 2 result: {}", task2.get());
    if (task3.isValid()) {
        Log("Task 3 result: {}", task3.get());
    }

    taskSystem.stopAndWait();

    Log("End");

    return EXIT_SUCCESS;
}