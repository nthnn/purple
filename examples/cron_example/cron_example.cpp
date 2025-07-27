#include <aetherium/cron/job.hpp>
#include <aetherium/cron/schedule.hpp>
#include <iostream>

using namespace Aetherium::Cron;

void task_1() {
    std::cout << timepoint_string(now())
        << " Task 1: Hello from my scheduled job!"
        << std::endl;
}

void task_2(const std::string& message) {
    std::cout << timepoint_string(now())
        << " Task 2: "
        << message
        << std::endl;
}

void task_3() {
    std::cout << timepoint_string(now())
        << " Task 3: This task runs every 10th minute of the hour."
        << std::endl;
}

void task_4() {
    std::cout << timepoint_string(now())
        << " Task 4: This task runs every minute from 0 to 5, 10 to 15, and at 30."
        << std::endl;
}

void task_5() {
    std::cout << timepoint_string(now())
        << " Task 5: This task runs every day at 00:00 UTC."
        << std::endl;
}

int main() {
    CronScheduler scheduler;
    std::cout << "Starting Cron Scheduler Example..."
        << std::endl;

    scheduler.add_job(
        "job1",
        "Every Minute Task", "* * * * *",
        task_1
    );

    scheduler.add_job(
        "job2",
        "Every 2 Minutes Task",
        "*/2 * * * *",
        std::bind(
            task_2,
            "This is a message from Task 2!"
        )
    );

    scheduler.add_job(
        "job3",
        "Every 10th Minute Task",
        "*/10 * * * *",
        task_3
    );

    scheduler.add_job(
        "job4",
        "Complex Minute Schedule Task",
        "0-5,10-15,30 * * * *",
        task_4
    );

    scheduler.add_job(
        "job5",
        "Daily Midnight Task",
        "0 0 * * *",
        task_5
    );

    scheduler.add_job(
        "invalid_job",
        "Invalid Cron Test", "60 * * * *",
        [](){
            std::cout << "This should not run."
                << std::endl;
        }
    );

    scheduler.start();

    scheduler.set_job_enabled("job1", false);
    std::cout << "Job 'job1' has been disabled."
        << std::endl;
    std::this_thread::sleep_for(CronSeconds(2));

    scheduler.set_job_enabled("job1", true);
    std::cout << "Job 'job1' has been re-enabled."
        << std::endl;
    std::this_thread::sleep_for(CronSeconds(2));

    scheduler.remove_job("job2");
    std::cout << "Job 'job2' has been removed."
        << std::endl;
    std::this_thread::sleep_for(CronSeconds(2));

    std::cout << "\n--- Current Jobs in Scheduler ---"
        << std::endl;

    for(const auto& job : scheduler.get_all_jobs())
        std::cout << "ID: " << job.id
                  << ", Desc: " << job.description
                  << ", Cron: " << job.schedule.get_cron_string()
                  << ", Next Run: " << timepoint_string(job.next_runtime)
                  << ", Enabled: " << (job.enabled ? "Yes" : "No")
                  << std::endl;
    std::cout << "---------------------------------"
        << std::endl;

    std::cout << "\nScheduler will continue running."
        << std::endl;

    for(int i = 0; i < 5; i++)
        std::this_thread::sleep_for(CronSeconds(5));

    return 0;
}
