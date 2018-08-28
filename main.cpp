#include "mbed.h"
#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif

// The example is to demonstrate CPU stats on Event Recorder
// CPU load is increased suddenly for single sample time to add an anomaly

#include "rtx_evr.h"

#if !defined(MBED_CPU_STATS_ENABLED) || !defined(DEVICE_LPTICKER) || !defined(DEVICE_SLEEP)
#error [NOT_SUPPORTED] test not supported
#endif

#define MbedDevStats             (0x81U)
#define MbedCpuStats              EventID(EventLevelAPI,    MbedDevStats, 0x01U)
#define MbedCpuLoad               EventID(EventLevelAPI,    MbedDevStats, 0x02U)

DigitalOut led1(LED1);

#define MAX_THREAD_STACK        384
#define SAMPLE_TIME             1000
#define INTERMEDIATE_TIME       5000

uint64_t prev_idle_time = 0;
int32_t wait_time = 5000;

void busy_thread()
{
    volatile uint64_t i = ~0;

    while(i--) {
        led1 = !led1;
        wait_us(wait_time);
    }
}

void cpu_stats_queue()
{
    mbed_stats_cpu_t stats;
    mbed_stats_cpu_get(&stats);

   (void)EventRecord2(MbedCpuStats, (uint32_t)stats.uptime, (uint32_t)(stats.idle_time - prev_idle_time));
    prev_idle_time = stats.idle_time;
}

void cpu_load_queue()
{
    mbed_stats_cpu_t stats;
    mbed_stats_cpu_get(&stats);

    uint64_t diff = (stats.idle_time - prev_idle_time);
    uint8_t idle = (diff * 100) / (SAMPLE_TIME*1000);    // usec;
    uint8_t usage = 100 - ((diff * 100) / (SAMPLE_TIME*1000));    // usec;;
    prev_idle_time = stats.idle_time;

   (void)EventRecord2(MbedCpuLoad, idle, usage);
}

int main()
{
    // Request the shared queue
    EventQueue *stats_queue = mbed_event_queue();
    Thread *thread;
    int id;

    {
        mbed_stats_sys_t stats;
        mbed_stats_sys_get(&stats);
        printf("Mbed OS Version: 0x%x \n", stats.os_version);
        printf("CPU ID: 0x%x \n", stats.cpu_id);
        printf("Compiler ID: %d \n", stats.compiler_id);
        printf("Compiler Version: %d \n", stats.compiler_version);
    }

    {
        mbed_stats_thread_t *stats = new mbed_stats_thread_t[5];
        int count = mbed_stats_thread_get_each(stats, 5);

        for(int i = 0; i < count; i++) {
            printf("ID: 0x%x \n", stats[i].id);
            printf("Name: %s \n", stats[i].name);
            printf("State: %d \n", stats[i].state);
            printf("Priority: %d \n", stats[i].priority);
            printf("Stack Size: %d \n", stats[i].stack_size);
            printf("Stack Space: %d \n", stats[i].stack_space);
            printf("\n");
        }
    }

    EventRecorderInitialize(EventRecordAPI, 1);  // initialize and start Event Recorder
    EventRecorderEnable (EventRecordAPI, MbedDevStats, MbedDevStats);
    EventRecorderStart();

    id = stats_queue->call_every(SAMPLE_TIME, cpu_stats_queue);

    for (int i = 0; i < 2; i++) {
        thread = new Thread(osPriorityNormal, MAX_THREAD_STACK);
        thread->start(busy_thread);

        // Suddenly increase the system load
        Thread::wait(INTERMEDIATE_TIME);
        wait_time -= 4500;
        Thread::wait(SAMPLE_TIME);
        wait_time += 4500;
        Thread::wait(INTERMEDIATE_TIME);

        stats_queue->cancel(id);
        id = stats_queue->call_every(SAMPLE_TIME, cpu_load_queue);
    }
    thread->terminate();
    stats_queue->cancel(id);
    return 0;
}
