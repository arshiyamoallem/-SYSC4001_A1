/**
 *
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 *
 */

#include<interrupts.hpp>

int main(int argc, char** argv) {

    //vectors is a C++ std::vector of strings that contain the address of the ISR
    //delays  is a C++ std::vector of ints that contain the delays of each device
    //the index of these elemens is the device number, starting from 0
    auto [vectors, delays] = parse_args(argc, argv);
    std::ifstream input_file(argv[1]);

    std::string trace;      //!< string to store single line of trace file
    std::string execution;  //!< string to accumulate the execution output

    /******************ADD YOUR VARIABLES HERE*************************/

        long long curr_sim_time = 0; // cumulative (in ms) time simulation clock variable

        int device_number = -1; // tracking I/O task being access in the device (-1 = no active device)

        // Interrupt handling values are measured in respect to the unit ms (miliseconds)
        const int SWITCH_MODE_TIME = 1; // switching to/from kernal and user mode
        const int SAVE_CONTEXT_TIME = 10; 
        const int VECTOR_FETCH_TIME = 1; // calculate where in memory the ISR start address is (locates the vector entry in the table)
        const int GET_ISR_ADDR_TIME = 1; 
        const int EXECUTE_ISR_TIME = 40;     
        const int EXECUTE_IRET_TIME = 1;

    /******************************************************************/

    //parse each line of the input trace file
    while(std::getline(input_file, trace)) {
        auto [activity, duration_intr] = parse_trace(trace);

        /******************ADD YOUR SIMULATION CODE HERE*************************/

        if (activity == "CPU") {
            curr_sim_time += duration_intr;
            execution += std::to_string(curr_sim_time) + ", " + std::to_string(duration_intr) + ", CPU burst\n";
        }

        else if (activity == "SYSCALL") {
            device_number = duration_intr;
            
            auto [boilerplate_exec, updated_time] = intr_boilerplate(curr_sim_time, device_number, SAVE_CONTEXT_TIME, vectors);
            execution += boilerplate_exec;
            curr_sim_time = updated_time;

            int device_io_delay_time = delays[device_number];
            
            execution += std::to_string(curr_sim_time) + ", " + std::to_string(EXECUTE_ISR_TIME) + ", SYSCALL: run the ISR (device driver)\n";
            curr_sim_time += EXECUTE_ISR_TIME;

            int device_data_transfer_time = std::min(40, device_io_delay_time);
            execution += std::to_string(curr_sim_time) + ", " + std::to_string(device_data_transfer_time) + ", transfer data from device to memory\n";
            curr_sim_time += device_data_transfer_time;

            int device_status_check_time = std::max(0, device_io_delay_time - device_data_transfer_time - EXECUTE_ISR_TIME); // get a non-negative value
            if (device_status_check_time > 0) { // ensures the assigned value is a non-negative number
                execution += std::to_string(curr_sim_time) + ", " + std::to_string(device_status_check_time) + ", check for errors\n";
                curr_sim_time += device_status_check_time;
            } 

            execution += std::to_string(curr_sim_time) + ", " + std::to_string(EXECUTE_IRET_TIME) + ", IRET for device " + std::to_string(device_number) + "\n";
            curr_sim_time += EXECUTE_IRET_TIME;

            execution += std::to_string(curr_sim_time) + ", " + std::to_string(SWITCH_MODE_TIME) + ", switch back to user mode\n";
            curr_sim_time += SWITCH_MODE_TIME;
        }

        else if (activity == "END_IO") {
            device_number = duration_intr;
            
            auto [boilerplate_exec, updated_time] = intr_boilerplate(curr_sim_time, device_number, SAVE_CONTEXT_TIME, vectors);
            execution += boilerplate_exec;
            curr_sim_time = updated_time;

            execution += std::to_string(curr_sim_time) + ", " + std::to_string(EXECUTE_ISR_TIME) +
                        ", ENDIO: run the ISR (device driver)\n";
            curr_sim_time += EXECUTE_ISR_TIME;

            int device_status_time = delays[device_number];
            execution += std::to_string(curr_sim_time) + ", " + std::to_string(device_status_time) +
                        ", check device status\n";
            curr_sim_time += device_status_time;

            execution += std::to_string(curr_sim_time) + ", " + std::to_string(EXECUTE_IRET_TIME) + ", IRET for device " + std::to_string(device_number) + "\n";
            curr_sim_time += EXECUTE_IRET_TIME;

            execution += std::to_string(curr_sim_time) + ", " + std::to_string(SWITCH_MODE_TIME) + ", switch back to user mode\n";
            curr_sim_time += SWITCH_MODE_TIME;
        }

        /************************************************************************/

    }

    input_file.close();

    write_output(execution);

    return 0;
}
