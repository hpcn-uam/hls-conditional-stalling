
#include "../raw_dep.hpp"

void get_id_stage(stream<packet_type> &in, stream<packet_type_id> &out ){

  packet_type_id out_elem;
  auto in_elem = in.read();

  out_elem.id = get_id(in_elem.tuple);
  out_elem.num = in_elem.num;
  out << out_elem;
}

void stall_stage(stream<packet_type_id> &in, stream<packet_type_id> &out ){
  #pragma HLS INTERFACE axis register_mode=both register port=in
  #pragma HLS INTERFACE axis register_mode=both register port=out
  #pragma HLS INTERFACE ap_ctrl_none port=return

  // Module state
  typedef ap_uint<ID_BITS+1> waitlist_id_t;
  static WaitList<waitlist_id_t,UPDATE_LATENCY> waitlist(ID_EMPTY);
  static packet_type_id in_elem;
  static bool conflict = false;
  #pragma HLS RESET variable=conflict
  #pragma HLS RESET variable=waitlist

  #pragma HLS PIPELINE II=1 style=frp
  int processed_samples = 0;
  START_SW_ONLY_LOOP(processed_samples < NUM_OF_SAMPLES)
    if (!conflict) in_elem = in.read();
    conflict = waitlist.is_in_list(in_elem.id);
    in_elem.valid = conflict? 0:1;
    out.write(in_elem);
    waitlist.update(conflict? waitlist_id_t(ID_EMPTY):
                          waitlist_id_t(in_elem.id));

    processed_samples+= in_elem.valid;//csim only (HLS compiler gets rid of it)
  END_SW_ONLY_LOOP

}


void raw_dep_stage(stream<packet_type_id> &in, stream<data_type> &out ){
  static data_type memory[MEMORY_SIZE]={0};
  #pragma HLS BIND_STORAGE variable=memory type=ram_s2p impl=bram latency=1

  static bool init_mem = true;
  #pragma HLS reset variable=init_mem
  if (init_mem) {
    init_loop:for (unsigned i = 0; i < MEMORY_SIZE; i++) {
      memory[i] = MEM_INIT;
    }
  }
  init_mem = false;

  ap_uint<NBITS(NUM_OF_SAMPLES)> processed_samples = 0;
  update: while(processed_samples<NUM_OF_SAMPLES) {
    #pragma HLS PIPELINE II=1
    #pragma HLS dependence variable=memory direction=RAW type=inter false
    packet_type_id in_elem = in.read();
    if (in_elem.valid == 1) {
      #pragma HLS LATENCY min=UPDATE_LATENCY max=UPDATE_LATENCY

      // the type of in_elem.id can have more bits than address_type
      // I do this because I want to test different id(addresses) bit sizes
      // without the frequency penalties of increassing the BRAM size
      address_type mem_addr = in_elem.id;
      memory[mem_addr] = update_logic(memory[mem_addr],in_elem.num);
      processed_samples++;
    }
  }
  // Memory flush
  flush: for(unsigned i = 0; i < MEMORY_SIZE; ++i) {
   // if using BRAM in read-first mode, only one operation is needed
    auto elem = memory[i];
    memory[i] = MEM_INIT;
    out<< elem;
  }

}

void raw_dep_core(stream<packet_type_id> &in, stream<data_type> &out ){
  #pragma HLS INTERFACE axis register_mode=both register port=in
  #pragma HLS INTERFACE axis register_mode=both register port=out
  #pragma HLS INTERFACE ap_ctrl_none port=return

  #pragma HLS DATAFLOW disable_start_propagation

  stream<packet_type_id,2> in_no_deps;
  stall_stage(in,in_no_deps);
  raw_dep_stage(in_no_deps, out );
}


void raw_dep(stream<packet_type> &in, stream<double> &out ){
  stream<packet_type_id> packet_with_id;
  START_SW_ONLY_LOOP(! in.empty())
    get_id_stage(in, packet_with_id );
  END_SW_ONLY_LOOP
  raw_dep_core(packet_with_id, out );
}
