#include "simlib.h"

// DEFINISI KONSTANTA
// Konstanta untuk event kedatangan penumpang
#define PERSON_ARRIVE_1     1   // di antrian 1
#define PERSON_ARRIVE_2     2   // di antrian 2
#define PERSON_ARRIVE_3     3   // di antrian 3

// Konstanta untuk event kedatangan dan kepergian bus
#define BUS_ARRIVE          4
#define BUS_DEPART          5

// Konstanta untuk event simulasi berakhir
#define SIMULATION_END      6

// Konstanta untuk number generator stream
// Stream untuk generate interarrival time pada setiap antrian
#define STREAM_QUEUE_1      1   // di antrian 1
#define STREAM_QUEUE_2      2   // di antrian 2
#define STREAM_QUEUE_3      3   // di antrian 3

// Stream untuk generate waktu unload dan load pada bus
#define STREAM_UNLOAD_BUS   4
#define STREAM_LOAD_BUS     5

// Stream untuk generate pilihan terminal tujuan (terminal 1 atau 2)
#define STREAM_PICK_DEST    6

// Deklarasi function
void arrive_person(int);        // Menangani event PERSON_ARRIVE
void depart_bus(void);          // Menangani event BUS_DEPART
void arrive_bus(void);          // Menangani event BUS_ARRIVE
double unload_bus(void);        // Menangani proses menurunkan penumpang dari bus
double load_bus(double);        // Menangani proses menaikkan penumpang ke bus
void report(void);              // Menampilkan statistik hasil simulasi

// Deklarasi variabel global
int bus_current_location;       // Menyimpan posisi bus terkini
int seats_taken;                // Menyimpan jumlah kursi bus yang terisi
int bus_do_loop;                // Menandakan apakah bus sudah mulai berkeliling
int bus_moving;                 // Menandakan apakah bus sedang bergerak atau diam
int person_current_id;

double mean_interarrival[4];    // Menyimpan rerata antar kedatangan pada setiap lokasi
double travel_time[4];          // Menyimpan waktu tempuh antar lokasi
double probability[3];          // Menyimpan peluang terpilihnya terminal 1 dan 2
double bus_stop_time;           // Menyimpan waktu bus tiba pada suatu lokasi
double loop_start_time;         // Menyimpan waktu bus memulai keliling

FILE *log_file, *stat_file;

int main() {
    log_file = fopen("rental_system_log.out", "w");
    stat_file = fopen("rental_system_stat.out", "w");

    travel_time[1] = 3600 / 30 * 1;
    travel_time[2] = 3600 / 30 * 4.5;
    travel_time[3] = 3600 / 30 * 4.5;

    mean_interarrival[1] = 3600 / 14;
    mean_interarrival[2] = 3600 / 10;
    mean_interarrival[3] = 3600 / 24;

    probability[1] = 0.583;
    probability[2] = 1.0;

    double simulation_duration = 3600 * 4;

    init_simlib();
    maxatr = 4;

    loop_start_time = sim_time;
    bus_do_loop = 0;
    bus_moving = 0;
    bus_stop_time = sim_time;

    bus_current_location = 3;
    person_current_id = 1;
    seats_taken = 0;

    event_schedule(simulation_duration, SIMULATION_END);

    event_schedule(expon(mean_interarrival[1], STREAM_QUEUE_1), PERSON_ARRIVE_1);
    event_schedule(expon(mean_interarrival[2], STREAM_QUEUE_2), PERSON_ARRIVE_2);
    event_schedule(expon(mean_interarrival[3], STREAM_QUEUE_3), PERSON_ARRIVE_3);

    event_schedule(sim_time, BUS_DEPART);

    for (int i = 1; i <= 6; ++i) {
        list_rank[i] = 1;
    }

    do {
        timing();
        switch (next_event_type) {
            case PERSON_ARRIVE_1:
                arrive_person(1);
                break;
            case PERSON_ARRIVE_2:
                arrive_person(2);
                break;
            case PERSON_ARRIVE_3:
                arrive_person(3);
                break;
            case BUS_ARRIVE:
                arrive_bus();
                break;
            case BUS_DEPART:
                depart_bus();
                break;
            case SIMULATION_END:
                report();
                break;
            default:
                break;
        }
    } while (next_event_type != SIMULATION_END);

    printf("Simulasi selesai.\n");

    fclose(log_file);
    fclose(stat_file);

    return 0;
}

void arrive_person(int current_location) {
    int destination;
    if (current_location == 3) {
        destination = random_integer(probability, STREAM_PICK_DEST);
    } else {
        destination = 3;
    }

    fprintf(log_file, "Penumpang <id:%d, src:%d, dest:%d> tiba di sistem pada %.0f.\n", person_current_id, current_location, destination, sim_time);

    transfer[1] = sim_time;
    transfer[4] = (double) destination;
    transfer[2] = (double) person_current_id;
    transfer[3] = (double) current_location;
    list_file(LAST, current_location);

    person_current_id++;
    double next_arrival = sim_time + expon(mean_interarrival[current_location], current_location);
    event_schedule(next_arrival, current_location);

    if (bus_current_location == current_location && !bus_moving) {
        double load_time = load_bus(0.0);
        double delta_time = sim_time + load_time - bus_stop_time;
        if (delta_time < 60 * 5) {
            event_cancel(BUS_DEPART);
            event_schedule(bus_stop_time + 60 * 5, BUS_DEPART);
        }
    }
}

void depart_bus() {
    int next_stop = (bus_current_location % 3) + 1;
    fprintf(log_file, "Bus berangkat dari lokasi %d pada %.0f.\n", bus_current_location, sim_time);

    if (bus_current_location == 3 && bus_do_loop)
    {
        double delta_time = sim_time - loop_start_time;
        loop_start_time = sim_time;
        sampst(delta_time, 10);
        fprintf(log_file, "1 Loop selama %.0f.\n", delta_time);
    }
    bus_do_loop = 1;

    if (bus_stop_time > 0) {
        double bus_stop_duration = sim_time - bus_stop_time;
        sampst(bus_stop_duration, bus_current_location + 3);
    }

    double arrival_time = sim_time + travel_time[bus_current_location];
    event_schedule(arrival_time, BUS_ARRIVE);
    bus_moving = 1;
}

void arrive_bus() {
    bus_moving = 0;
    bus_stop_time = sim_time;

    int departure_location = bus_current_location;
    bus_current_location = (bus_current_location % 3) + 1;
    fprintf(log_file, "Bus tiba di lokasi %d pada %.0f.\n", bus_current_location, sim_time);

    double delta_time = 0.0;
    delta_time = unload_bus();
    delta_time += load_bus(delta_time);

    if (delta_time < 60 * 5) {
        event_cancel(BUS_DEPART);
        event_schedule(sim_time + 60 * 5, BUS_DEPART);
    }
}

double unload_bus() {
    double unload_time = 0.0;
    while (list_size[bus_current_location + 3] > 0) {
        unload_time += uniform(16.0, 24.0, STREAM_LOAD_BUS);

        list_remove(FIRST, bus_current_location + 3);
        fprintf(log_file, "Penumpang <id:%d, src:%d, dest:%d> turun dari bus pada %.0f\n", (int)transfer[2], (int)transfer[3], (int)transfer[4], sim_time + unload_time);

        double person_system_duration = sim_time - transfer[1];
        sampst(person_system_duration, transfer[3] + 6);

        seats_taken--;
        timest((double) seats_taken, 1);

        event_cancel(BUS_DEPART);
        event_schedule(sim_time + unload_time, BUS_DEPART);
    }
    return unload_time;
}

double load_bus(double unload_time) {
    double delta_time = 0.0;
    double load_time = 0.0;
    double delay = 0.0;

    while(list_size[bus_current_location] > 0 && seats_taken < 20){
        load_time += uniform(15.0, 25.0, STREAM_LOAD_BUS);
        delta_time = unload_time + load_time;

        list_remove(FIRST, bus_current_location);
        fprintf(log_file, "Penumpang <id:%d, src:%d, dest:%d> naik bus pada %.0f.\n", (int)transfer[2], (int)transfer[3], (int)transfer[4], sim_time + delta_time);

        delay = sim_time + delta_time - transfer[1];
        sampst(delay, bus_current_location);

        list_file(LAST, transfer[4] + 3);
        seats_taken++;
        timest((double) seats_taken, 1);

        event_cancel(BUS_DEPART);
        event_schedule(sim_time + delta_time, BUS_DEPART);
    }
    return delta_time;
}

void report() {
    fprintf(stat_file, "a. Jumlah penumpang dalam setiap antrian:\n");
    for (int i = 1; i <= 3; i++) {
        filest(i);
        fprintf(stat_file, "\tAntrian %d:\n", i);
        fprintf(stat_file, "\t\tRata-rata: %.0f\n", transfer[1]);
        fprintf(stat_file, "\t\tMaksimum: %.0f\n", transfer[2]);
    }
    fprintf(stat_file, "\n");
    
    fprintf(stat_file, "b. Delay dalam setiap antrian:\n");
    for (int i = 1; i <= 3; i++) {
        sampst(0.0, -i);
        fprintf(stat_file, "\tAntrian %d:\n", i);
        fprintf(stat_file, "\t\tRata-rata: %5.2f\n", transfer[1]);
        fprintf(stat_file, "\t\tMaksimum: %5.2f\n", transfer[3]);
    }
    fprintf(stat_file, "\n");
    
    fprintf(stat_file, "c. Jumlah penumpang dalam bus:\n");
    timest(0.0, -1);
    fprintf(stat_file, "\tRata-rata: %.0f\n", transfer[1]);
    fprintf(stat_file, "\tMaksimum: %.0f\n", transfer[2]);
    fprintf(stat_file, "\n");

    fprintf(stat_file, "d. Durasi bus berada di setiap lokasi:\n");
    for (int i = 1; i <= 3; i++) {
        sampst(0.0, -i - 3);
        fprintf(stat_file, "\tAntrian %d:\n", i);
        fprintf(stat_file, "\t\tRata-rata: %5.2f\n", transfer[1]);
        fprintf(stat_file, "\t\tMaksimum: %5.2f\n", transfer[3]);
        fprintf(stat_file, "\t\tMinimum: %5.2f\n", transfer[4]);
    }
    fprintf(stat_file, "\n");
    
    fprintf(stat_file, "e. Waktu tempuh bus untuk melakukan satu keliling:\n");
    sampst(0.0, -10);
    fprintf(stat_file, "\tRata-rata: %.0f\n", transfer[1]);
    fprintf(stat_file, "\tMaksimum: %.0f\n", transfer[3]);
    fprintf(stat_file, "\tMinimum: %.0f\n", transfer[4]);
    fprintf(stat_file, "\n");

    fprintf(stat_file, "f. Durasi penumpang berada di sistem:\n");
    for (int i = 1; i <= 3; i++) {
        sampst(0.0, -i - 6);
        fprintf(stat_file, "\tPenumpang dari antrian %d:\n", i);
        fprintf(stat_file, "\t\tRata-rata: %5.2f\n", transfer[1]);
        fprintf(stat_file, "\t\tMaksimum: %5.2f\n", transfer[3]);
        fprintf(stat_file, "\t\tMinimum: %5.2f\n", transfer[4]);
    }
}