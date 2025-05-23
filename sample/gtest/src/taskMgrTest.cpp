#include <iostream>
#include <gtest/gtest.h>
#include "callable.h"
#include "taskMgr.h"

using namespace vcDynTimer;
TEST(taskMgr, executeAll)
{
    TaskMgr tm;
    auto [id, fut] = tm.addTask(TaskMode::once, 1000, 0, static_cast<int (*)()>(print_hello));
    tm.start();
    tm.control(id, TaskControl::start);
    ASSERT_EQ(1, fut.get());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "taskMgr.executeAll finished!!!" << std::endl;
}