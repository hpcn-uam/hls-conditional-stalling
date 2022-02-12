
#include "../raw_dep.hpp"

void get_id_stage(stream<packet_type> &in, stream<packet_type_id> &out ){
  packet_type_id out_elem;
  auto in_elem = in.read();

  out_elem.id = get_id(in_elem.tuple);
  out_elem.num = in_elem.num;
  out << out_elem;
}


void raw_dep_core(stream<packet_type_id> &in, stream<double> &out ){
  #pragma HLS INTERFACE axis register_mode=both register port=in
  #pragma HLS INTERFACE axis register_mode=both register port=out
  #pragma HLS INTERFACE ap_ctrl_none port=return

  static data_type memory[MEMORY_SIZE];
  #pragma HLS BIND_STORAGE variable=memory type=ram_s2p impl=bram latency=1
  static bool init_mem = true;
  #pragma HLS reset variable=init_mem

  if (init_mem) {
    init_loop:for (unsigned i = 0; i < MEMORY_SIZE; i++) {
      memory[i] = MEM_INIT;
    }
  }
  init_mem = false;

  //Conditional stalling
  constexpr unsigned int LOOP_LATENCY = UPDATE_LATENCY+2;

  typedef ap_uint<ID_BITS+1> waitlist_id_t;
  WaitList<waitlist_id_t,UPDATE_LATENCY> waitlist(ID_EMPTY);
  bool get_new_input = true;
  packet_type_id in_elem;

  int processed_samples = 0;
  update: while(processed_samples<NUM_OF_SAMPLES) {
    #pragma HLS PIPELINE II=1
    #pragma HLS dependence variable=memory direction=RAW type=inter false
	#pragma HLS LATENCY min=LOOP_LATENCY
    if (get_new_input) {
      in_elem = in.read();
    }

    waitlist_id_t new_in_pipeline;
    if(waitlist.is_in_list(in_elem.id)){
      new_in_pipeline = ID_EMPTY;
      get_new_input = false;
    }else{
      new_in_pipeline = in_elem.id;
      get_new_input = true;
      {
        #pragma HLS LATENCY min=UPDATE_LATENCY max=UPDATE_LATENCY

        // the type of in_elem.id can have more bits than address_type
        // I do this because I want to test different id(addresses) bit sizes
        // without the frequency penalties of increassing the BRAM size
        address_type mem_addr = in_elem.id;
        memory[mem_addr] = update_logic(memory[mem_addr],in_elem.num);
        processed_samples++;
      }
    }

    waitlist.update(new_in_pipeline);
  }

  // Memory flush
  flush: for(unsigned i = 0; i < MEMORY_SIZE; ++i) {
   // if using BRAM in read-first mode, only one operation is needed
    auto elem = memory[i];
    memory[i] = 0;
    out<< elem;
  }

}


void raw_dep(stream<packet_type> &in, stream<double> &out ){
  stream<packet_type_id> packet_with_id;
  START_SW_ONLY_LOOP(! in.empty())
    get_id_stage(in, packet_with_id );
  END_SW_ONLY_LOOP
  raw_dep_core(packet_with_id, out );
}
