
    void pac_log_start();
    void pac_log_end();
    int benchmark_sample_program();
    void initialise_benchmark_sample_program();
    int main() {
        pac_log_start();
        initialise_benchmark_sample_program();
        int r=benchmark_sample_program();
        pac_log_end();
        return 0;
    }
    