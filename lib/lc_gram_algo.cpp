//
// Created by Diego Diaz on 4/7/20.
//

#include "lc_gram_algo.hpp"
#include <cmath>
#include <thread>

void compress_dictionary(dictionary &dict, vector_t &sa, gram_info_t &p_gram,
                         bvb_t &r_lim, ivb_t &rules, phrase_map_t &mp_map) {

    /*size_t pos, prev_pos, lcs, l_sym, prev_l_sym, dummy_sym = dict.alphabet+1, rank=0;
    //remove unnecessary entries from the SA to
    // avoid dealing with corner cases
    size_t k=0;
    for(size_t j=0;j<sa.size();j++){
        if(sa[j]!=0){
            pos = sa[j]-1;
            //the SA position represents either
            // i) a suffix of length >= 2
            // ii) phrase of length 1
            if(!dict.d_lim[pos] || // pattern 0...1 means i)
               (dict.d_lim[pos] && (pos==0 || dict.d_lim[pos-1]))){ //pattern 11 means ii)
                sa[k++] = sa[j];
            }
        }
    }
    sa.resize(k);

    size_t width = sdsl::bits::hi(dict.alphabet+dict.dict.size()-dict.n_phrases)+1;
    vector_t ranks(dict.n_phrases, 0, width);

    bv_rs_t d_lim_rs(&dict.d_lim);
    phrase_map_t new_phrases_ht;
    string_t phrase(2, dict.dict.width());

    bool is_maximal, exists_as_rule;
    size_t i=1, run_bg, rem_elms=0, new_phrases=0, existing_phrases=0;
    std::vector<size_t> phrases_sa_ranges;
    phrases_sa_ranges.reserve(dict.n_phrases);

    while(i<sa.size()) {

        assert(sa[i]!=0);
        prev_pos = sa[i - 1] - 1;
        if(prev_pos==0 || dict.d_lim[prev_pos - 1]){
            exists_as_rule = true;
            ranks[d_lim_rs(prev_pos)] = rank+1;
            prev_l_sym = dummy_sym;
        }else{
            exists_as_rule = false;
            prev_l_sym = dict.dict[prev_pos - 1];
        }

        is_maximal = false;
        run_bg = i-1;

        do {
            pos = sa[i]-1;
            lcs = 0;
            bool limit_a, limit_b, equal;

            do {
                equal = dict.dict[pos+lcs] == dict.dict[prev_pos+lcs];
                limit_a = dict.d_lim[pos+lcs] || dict.dict[pos+lcs]>dict.max_sym;
                limit_b = dict.d_lim[prev_pos+lcs] || dict.dict[prev_pos+lcs]>dict.max_sym;
                lcs++;
            }while(equal && !limit_a && !limit_b);

            if((equal+limit_a+limit_b)==3) {
                if (pos==0 || dict.d_lim[pos - 1]) {
                    ranks[d_lim_rs(pos)] = rank+1;
                    exists_as_rule = true;
                    l_sym = dummy_sym;
                } else {
                    l_sym = dict.dict[pos - 1];
                    if (!is_maximal && prev_l_sym != dummy_sym) {
                        is_maximal = lcs>1 && (prev_l_sym != l_sym);
                    }
                }
                prev_pos = pos;
                if (l_sym != dummy_sym) prev_l_sym = l_sym;
            }else{
                lcs=0;
            }
            i++;
        } while (i<sa.size() && lcs!=0);

        if(exists_as_rule || is_maximal) {

            if(!exists_as_rule){
                new_phrases++;
            }else{
                existing_phrases++;
            }

            assert(run_bg<=i-2);
            if(lcs!=0) assert(i==sa.size());

            size_t run_end = lcs==0 ? i-2 : i-1;
            phrases_sa_ranges.emplace_back(run_bg-rem_elms);

            if((run_end-run_bg+1)>1) {

                //put the phrase in reverse order
                size_t tmp_pos = sa[run_bg]-1;
                size_t tmp_pos2 = tmp_pos;
                phrase.clear();
                while(!dict.d_lim[tmp_pos]) tmp_pos++;
                do{
                    phrase.push_back(dict.dict[tmp_pos--]);
                }while(tmp_pos>=tmp_pos2);

                //hash only those phrases that can be proper
                // suffixes of other phrases
                if(phrase.size()>1){
                    phrase.mask_tail();
                    auto res = new_phrases_ht.insert(phrase.data(), phrase.n_bits(), rank+1);
                    assert(res.second);
                }
                //invalidate the extra occurrences of the phrase
                for(size_t j=run_bg+1;j<=run_end;j++) sa[j] = 0;
                rem_elms+=run_end-run_bg;
            }

            rank++;
        }
        //
    }

    //corner case
    if(lcs==0){
        if(sa[i-1]==0 || dict.d_lim[sa[i-1]-2]){
            ranks[d_lim_rs(sa[i-1]-1)] = rank+1;
            phrases_sa_ranges.emplace_back((i-1)-rem_elms);
        }
    }

    k=0;
    size_t j=0;
    while(j<sa.size()){
        if(sa[j]!=0) sa[k++] = sa[j];
        j++;
    }
    sa.resize(k);*/

    bv_t phr_marks(dict.dict.size(), false);
    string_t phrase(2, sdsl::bits::hi(dict.alphabet)+1);
    bool is_maximal, exist_as_phrase;
    size_t u=0, d_pos, pl_sym, bg_pos, rank=1, l_sym, dummy_sym = dict.alphabet+1, f_sa_pos;

    bv_rs_t d_lim_rs(&dict.d_lim);
    size_t width = sdsl::bits::hi(dict.alphabet+dict.dict.size()-dict.n_phrases)+1;
    vector_t ranks(dict.n_phrases, 0, width);
    phrase_map_t new_phrases_ht;

    while(u<sa.size()) {
        d_pos = (sa[u]>>1UL) - 1UL;

        if(dict.d_lim[d_pos]){
            u++;
            while(u<sa.size() && sa[u] & 1UL){
                u++;
            }
        }else{
            f_sa_pos = d_pos;
            bg_pos = u;


            is_maximal = false;
            pl_sym = (d_pos==0 || dict.d_lim[d_pos-1]) ? dummy_sym : dict.dict[d_pos-1];
            if(pl_sym==dummy_sym){
                exist_as_phrase = true;
                ranks[d_lim_rs(d_pos)] = rank;
            }else{
                exist_as_phrase = false;
            }

            u++;
            while(u<sa.size() && sa[u] & 1UL){
                d_pos = (sa[u]>>1UL) - 1;
                l_sym = d_pos==0 || dict.d_lim[d_pos-1] ? dummy_sym : dict.dict[d_pos-1];
                if(!is_maximal && l_sym!=pl_sym) is_maximal = true;
                if(!exist_as_phrase && l_sym==dummy_sym){
                    ranks[d_lim_rs(d_pos)] = rank;
                    exist_as_phrase = true;
                }
                pl_sym = l_sym;
                u++;
            }

            if(is_maximal || exist_as_phrase){
                if(u-bg_pos>1){
                    //put the phrase in reverse order
                    size_t tmp_pos = f_sa_pos;
                    phrase.clear();
                    while(!dict.d_lim[tmp_pos]) tmp_pos++;

                    for(size_t k=tmp_pos+1;k-->f_sa_pos;){
                        phrase.push_back(dict.dict[k]);
                    }

                    phrase.mask_tail();
                    auto res = new_phrases_ht.insert(phrase.data(), phrase.n_bits(), rank);
                    assert(res.second);

                    for(size_t j=bg_pos;j<u;j++){
                        phr_marks[(sa[j]>>1UL)-1] = true;
                    }
                }

                sa[rank-1] = f_sa_pos;
                rank++;
            }
        }
    }

    sa.resize(rank-1);
    new_phrases_ht.shrink_databuff();


    //assign the ranks to the original phrases
    size_t j=0;
    for(auto const& ptr : mp_map){
        phrase_map_t::val_type val=0;
        mp_map.get_value_from(ptr, val);

        //TODO testing
        if(ranks[j]==0){
            std::cout<< j <<" "<<mp_map.size()<<" "<<ranks.size()<<" "<<sa.size()<<std::endl;
            size_t tmp=0, l=0;
            while(tmp<j){
                if(dict.d_lim[l]) tmp++;
                l++;
            }
            std::cout<<dict.dict[l-1]<<" "<<dict.dict[l]<<" "<<dict.dict[l+1]<<std::endl;
            std::cout<<dict.d_lim[l-1]<<" "<<dict.d_lim[l]<<" "<<dict.d_lim[l+1]<<std::endl;
            do{
                std::cout<<dict.dict[l]<<" ";
            }while(!dict.d_lim[l++]);
            std::cout<<""<<std::endl;
        }
        //

        assert(ranks[j]>0);
        val |= (dict.max_sym+ranks[j++])<<1UL;
        mp_map.insert_value_at(ptr, val);
    }
    sdsl::util::clear(ranks);
    sdsl::util::clear(d_lim_rs);

    //collapse the full dictionary in the grammar
    size_t em_nt;
    size_t pos;
    for(auto const &sa_locus : sa) {
        pos = sa_locus;

        rules.push_back(dict.min_sym+dict.dict[pos]);
        r_lim.push_back(false);
        pos++;

        while(!phr_marks[pos] && !dict.d_lim[pos]){
            rules.push_back(dict.min_sym+dict.dict[pos]);
            r_lim.push_back(false);
            pos++;
        }

        if(phr_marks[pos]){
            size_t tmp_pos = pos;
            while(!dict.d_lim[pos]) pos++;

            phrase.clear();
            for(size_t k=pos+1;k-->tmp_pos;){
                phrase.push_back(dict.dict[k]);
            }
            phrase.mask_tail();
            auto res = new_phrases_ht.find(phrase.data(), phrase.n_bits());
            assert(res.second);
            em_nt = 0;
            new_phrases_ht.get_value_from(res.first, em_nt);

            rules.push_back(dict.max_sym+em_nt);
        }else{
            assert(dict.d_lim[pos]);
            rules.push_back(dict.min_sym+dict.dict[pos]);
        }
        r_lim.push_back(true);

        /*
        //extract the greatest proper suffix of the phrase
        // in reverse order
        size_t tmp_pos = pos;
        while(!dict.d_lim[pos]){
            pos++;
        }
        p_len = pos-tmp_pos+1;

        phrase.clear();
        while(pos>tmp_pos){
            phrase.push_back(dict.dict[pos--]);
            p_len--;
        }

        //check if the proper suffixes exists as phrases
        found = false;
        while(phrase.size()>1){
            phrase.mask_tail();
            auto res = new_phrases_ht.find(phrase.data(), phrase.n_bits());
            if(res.second){
                em_nt = 0;
                new_phrases_ht.get_value_from(res.first, em_nt);
                found = true;
                break;
            }
            phrase.pop_back();
            p_len++;
        }

        if(!found){
            while(!dict.d_lim[tmp_pos]){
                rules.push_back(dict.min_sym+dict.dict[tmp_pos++]);
                r_lim.push_back(false);
            }

            rules.push_back(dict.min_sym+dict.dict[tmp_pos]);
            r_lim.push_back(true);
            k++;
        }else{
            j = 0;
            while(j<p_len){
                rules.push_back(dict.min_sym+dict.dict[tmp_pos+j]);
                r_lim.push_back(false);
                j++;
            }
            assert(em_nt>0);
            rules.push_back(dict.max_sym+em_nt);
            r_lim.push_back(true);
            k++;
        }*/
    }
    p_gram.rules_breaks.push_back(sa.size());
    p_gram.r += sa.size();
    //exit(0);
}

void assign_ids(phrase_map_t &mp_map, ivb_t &r, bvb_t &r_lim, dictionary &dict, gram_info_t &p_gram,
                sdsl::cache_config &config) {


    vector_t sa(dict.dict.size(), 0, sdsl::bits::hi(dict.dict.size())+2);
    uint8_t width = sdsl::bits::hi(dict.dict.size())+1;
    std::cout<<"      Sorting the dictionary using suffix induction"<<std::endl;
    if(width<=8){
        suffix_induction<uint8_t>(sa, dict);
    }else if(width<=16){
        suffix_induction<uint16_t>(sa, dict);
    }else if(width<=32){
        suffix_induction<uint32_t>(sa, dict);
    }else{
        suffix_induction<uint64_t>(sa, dict);
    }

    //TODO testing
    /*for(auto && i : sa){
        if(i!=0){
            size_t pos = i-1;
            do{
                std::cout<<dict.dict[pos]<<" ";
            }while(!dict.d_lim[pos++]);
            std::cout<<""<<std::endl;
        }
    }*/
    //
    compress_dictionary(dict, sa, p_gram, r_lim, r, mp_map);
}

void join_parse_chunks(const std::string &output_file, std::vector<std::string> &chunk_files) {

    //concatenate the files
    std::ofstream of(output_file, std::ofstream::binary);
    size_t buff_size = BUFFER_SIZE/sizeof(size_t);
    size_t len, rem, to_read, start, end;
    auto *buffer = new size_t[buff_size];

    for(auto const& file: chunk_files){

        std::ifstream i_file(file, std::ifstream::binary);

        i_file.seekg (0, std::ifstream::end);
        len = i_file.tellg()/sizeof(size_t);
        i_file.seekg (0, std::ifstream::beg);

        rem=len;
        to_read = std::min<size_t>(buff_size, len);

        while(true){

            i_file.seekg( (rem - to_read) * sizeof(size_t));
            i_file.read((char *)buffer, sizeof(size_t)*to_read);
            assert(i_file.good());

            //invert data
            start =0;end=to_read-1;
            while(start<end){
                std::swap(buffer[start++], buffer[end--]);
            }

            of.write((char *)buffer, sizeof(size_t)*to_read);
            assert(of.good());

            rem -= i_file.gcount()/sizeof(size_t);
            to_read = std::min<size_t>(buff_size, rem);
            if(to_read == 0) break;
        }
        i_file.close();

        if(remove(file.c_str())){
            std::cout<<"Error trying to remove temporal file"<<std::endl;
            std::cout<<"Aborting"<<std::endl;
            exit(1);
        }
    }
    delete[] buffer;
    of.close();
}

size_t join_thread_phrases(phrase_map_t& map, std::vector<std::string> &files) {

    bool rep;
    size_t dic_bits=0;

    for(auto const& file : files){

        std::ifstream text_i(file, std::ios_base::binary);

        text_i.seekg (0, std::ifstream::end);
        size_t tot_bytes = text_i.tellg();
        text_i.seekg (0, std::ifstream::beg);

        auto buffer = reinterpret_cast<char *>(malloc(tot_bytes));

        text_i.read(buffer, std::streamsize(tot_bytes));

        bitstream<ht_buff_t> bits;
        bits.stream = reinterpret_cast<ht_buff_t*>(buffer);

        size_t next_bit = 32;
        size_t tot_bits = tot_bytes*8;
        size_t key_bits;
        void* key=nullptr;
        size_t max_key_bits=0;

        while(next_bit<tot_bits){

            key_bits = bits.read(next_bit-32, next_bit-1);

            size_t n_bytes = INT_CEIL(key_bits, bitstream<ht_buff_t>::word_bits)*sizeof(ht_buff_t);
            if(key_bits>max_key_bits){
                if(key==nullptr){
                    key = malloc(n_bytes);
                }else {
                    key = realloc(key, n_bytes);
                }
                max_key_bits = key_bits;
            }

            char *tmp = reinterpret_cast<char*>(key);
            tmp[INT_CEIL(key_bits, 8)-1] = 0;

            bits.read_chunk(key, next_bit, next_bit+key_bits-1);
            next_bit+=key_bits;
            rep = bits.read(next_bit, next_bit);
            next_bit+=33;

            auto res = map.insert(key, key_bits, rep);
            if(!res.second){
                map.insert_value_at(res.first, 1UL);
            }else{
                dic_bits+=key_bits;
            }
        }
        text_i.close();

        if(remove(file.c_str())){
            std::cout<<"Error trying to remove temporal file"<<std::endl;
            std::cout<<"Aborting"<<std::endl;
            exit(1);
        }
        free(key);
        free(buffer);
    }
    map.shrink_databuff();
    return dic_bits;
}


template<template<class, class> class lc_parser_t>
void build_lc_gram(std::string &i_file, size_t n_threads, size_t hbuff_size,
                   gram_info_t &p_gram, /*alpha_t alphabet,*/ sdsl::cache_config &config) {

    std::cout<<"  Generating a locally consistent grammar:    "<<std::endl;
    std::string output_file = sdsl::cache_file_name("tmp_output", config);
    std::string tmp_i_file = sdsl::cache_file_name("tmp_input", config);

    /*
    // given an index i in symbol_desc
    //0 symbol i is in alphabet is unique
    //>2 symbol i is sep symbol
    sdsl::int_vector<2> symbol_desc(alphabet.back().first+1,0);
    for(auto & sym : alphabet){
        symbol_desc[sym.first] = sym.second > 1;
    }
    symbol_desc[alphabet[0].first]+=2;*/

    ivb_t rules(p_gram.rules_file, std::ios::out, BUFFER_SIZE);
    bvb_t rules_lim(p_gram.rules_lim_file, std::ios::out);
    for(size_t i=0;i<p_gram.r; i++){
        rules.push_back(i);
        rules_lim.push_back(true);
    }
    for(auto const& pair : p_gram.sym_map){
        rules[pair.first] = pair.first;
    }

    size_t iter=1;
    size_t rem_phrases;

    typedef lc_parser_t<i_file_stream<uint8_t>, string_t> byte_parser_t;
    typedef lc_parser_t<i_file_stream<size_t>, string_t> int_parser_t;

    std::cout<<"    Parsing round "<<iter++<<std::endl;
    rem_phrases = build_lc_gram_int<byte_parser_t>(i_file, tmp_i_file,
                                             n_threads, hbuff_size,
                                             p_gram, rules, rules_lim, config);
    while (rem_phrases > 0) {
        std::cout<<"    Parsing round "<<iter++<<std::endl;
        rem_phrases = build_lc_gram_int<int_parser_t>(tmp_i_file, output_file,
                                                      n_threads, hbuff_size,
                                                      p_gram, rules, rules_lim, config);
        remove(tmp_i_file.c_str());
        rename(output_file.c_str(), tmp_i_file.c_str());
    }

    //sdsl::util::clear(symbol_desc);

    {//put the compressed string at end
        std::ifstream c_vec(tmp_i_file, std::ifstream::binary);
        c_vec.seekg(0, std::ifstream::end);
        size_t tot_bytes = c_vec.tellg();
        c_vec.seekg(0, std::ifstream::beg);
        auto *buffer = reinterpret_cast<size_t*>(malloc(BUFFER_SIZE));
        size_t read_bytes =0;
        p_gram.c=0;
        while(read_bytes<tot_bytes){
            c_vec.read((char *) buffer, BUFFER_SIZE);
            read_bytes+=c_vec.gcount();
            assert((c_vec.gcount() % sizeof(size_t))==0);
            for(size_t i=0;i<c_vec.gcount()/sizeof(size_t);i++){
                rules.push_back(buffer[i]);
                rules_lim.push_back(false);
                p_gram.c++;
            }
        }
        rules_lim[rules_lim.size() - 1] = true;
        p_gram.r++;
        c_vec.close();
        free(buffer);
    }
    p_gram.g = rules.size();

    p_gram.n_p_rounds = p_gram.rules_breaks.size();
    std::vector<size_t> rule_breaks;
    rule_breaks.push_back(p_gram.max_tsym+1);
    for(unsigned long i : p_gram.rules_breaks){
        rule_breaks.push_back(rule_breaks.back()+i);
    }
    std::swap(p_gram.rules_breaks, rule_breaks);

    rules.close();
    rules_lim.close();

    std::cout<<"  Locally consistent grammar finished"<<std::endl;
    std::cout<<"    Stats:"<<std::endl;
    std::cout<<"      Number of terimnals:    "<<(int)p_gram.sigma<<std::endl;
    std::cout<<"      Number of nonterminals: "<<p_gram.rules_breaks.back()-(p_gram.max_tsym+1)+1<<std::endl;
    std::cout<<"      Grammar size:           "<<p_gram.g<<std::endl;
    std::cout<<"      Compressed string:      "<<p_gram.c<<std::endl;

    if(remove(tmp_i_file.c_str())){
        std::cout<<"Error trying to delete file "<<tmp_i_file<<std::endl;
    }
}

template<class parser_t, class out_sym_t>
size_t build_lc_gram_int(std::string &i_file, std::string &o_file,
                         size_t n_threads, size_t hbuff_size,
                         gram_info_t &p_gram, ivb_t &rules,
                         bvb_t &rules_lim,
                         sdsl::cache_config &config) {

    typedef typename parser_t::sym_type          sym_type;
    typedef typename parser_t::stream_type       stream_type;
    typedef parse_data_t<stream_type, out_sym_t> parse_data_type;

    size_t min_sym = p_gram.rules_breaks.empty() ? 0 : p_gram.r - p_gram.rules_breaks.back();
    size_t max_sym = p_gram.r-1;

    parser_t parser(p_gram.suf_pos, min_sym, max_sym, p_gram.sep_tsym);

    phrase_map_t mp_table(0, "", 0.8);

    auto thread_ranges = parser.partition_text(n_threads, i_file);

    std::vector<parse_data_type> threads_data;
    threads_data.reserve(thread_ranges.size());

    //how many size_t cells we can fit in the buffer
    size_t buff_cells = hbuff_size/sizeof(size_t);

    //number of bytes per thread
    size_t hb_bytes = (buff_cells / thread_ranges.size()) * sizeof(size_t);

    void *buff_addr = malloc(hbuff_size);
    auto tmp_addr = reinterpret_cast<char*>(buff_addr);

    size_t k=0;
    for(auto &range : thread_ranges) {
        std::string tmp_o_file = o_file.substr(0, o_file.size() - 5);
        tmp_o_file.append("_range_"+std::to_string(range.first)+"_"+std::to_string(range.second));
        threads_data.emplace_back(i_file, tmp_o_file, mp_table,
                                  range.first, range.second,
                                  hb_bytes, tmp_addr + (k*hb_bytes));
        k++;
    }

    std::cout<<"      Computing the phrases in the text"<<std::flush;
    {
        std::vector<std::thread> threads(threads_data.size());
        hash_functor<parse_data_type, parser_t> hf;
        for(size_t i=0;i<threads_data.size();i++){
            threads[i] = std::thread(hf, std::ref(threads_data[i]), std::ref(parser));
        }

        for(size_t i=0;i<threads_data.size();i++) {
            threads[i].join();
        }
    }
    free(buff_addr);

    //join the different phrase files
    std::vector<std::string> phrases_files;
    for(size_t i=0;i<threads_data.size();i++){
        phrases_files.push_back(threads_data[i].thread_dict.dump_file());
    }
    size_t dict_bits = join_thread_phrases(mp_table, phrases_files);

    size_t psize=0;//<- for the iter stats
    if(mp_table.size()>0){

        size_t width = sdsl::bits::hi(p_gram.r+1)+1;
        size_t dict_syms = dict_bits/width;

        //p_gram.rules_breaks.push_back(mp_table.size());
        const bitstream<ht_buff_t>& stream = mp_table.get_data();
        key_wrapper key_w{width, mp_table.description_bits(), stream};

        //temporal unload of the hash table (not the data)
        std::string st_table = sdsl::cache_file_name("ht_data", config);
        mp_table.unload_table(st_table);

        //create a dictionary from where the ids will be computed
        dictionary dict(mp_table, min_sym, max_sym, key_w, dict_syms);

        //rename phrases according to their lexicographical ranks
        std::cout<<"      Assigning identifiers to the phrases"<<std::endl;
        assign_ids(mp_table, rules, rules_lim, dict, p_gram, config);

        //reload the hash table
        mp_table.load_table(st_table);
        if(remove(st_table.c_str())){
            std::cout<<"Error trying to remove temporal file"<<std::endl;
            std::cout<<"Aborting"<<std::endl;
            exit(1);
        }

        std::cout<<"      Creating the parse of the text"<<std::flush;
        {//store the phrases into a new file
            std::vector<std::thread> threads(threads_data.size());
            parse_functor<parse_data_type, parser_t> pf;
            for(size_t i=0;i<threads_data.size();i++){
                threads[i] = std::thread(pf, std::ref(threads_data[i]), std::ref(parser));
            }

            for(size_t i=0;i<threads_data.size();i++) {
                threads[i].join();
            }
        }

        std::vector<std::string> chunk_files;
        for(size_t i=0;i<threads_data.size();i++){
            chunk_files.push_back(threads_data[i].ofs.file);
        }
        join_parse_chunks(o_file, chunk_files);

        //compute the new position of the suffixes
        size_t acc=0, j=0;
        for(size_t i=0;i<threads_data.size();i++){
            for(size_t u=0;u<threads_data[i].new_suf_pos.size();u++){
                threads_data[i].new_suf_pos[u]  = acc + ( (threads_data[i].n_phrases-1) -  threads_data[i].new_suf_pos[u]);
            }
            for(size_t u= threads_data[i].new_suf_pos.size(); u-->0;){
                p_gram.suf_pos[j++] = threads_data[i].new_suf_pos[u];
            }
            std::vector<size_t>().swap(threads_data[i].new_suf_pos);
            acc += threads_data[i].n_phrases;
        }

        {// this is just to get the size of the resulting parse
            i_file_stream<size_t> ifs(o_file, BUFFER_SIZE);
            psize = ifs.tot_cells;
        }

        std::cout<<"      Stats:"<<std::endl;
        std::cout<<"        Parse size:          "<<psize<<std::endl;
        std::cout<<"        New nonterminals:    "<<p_gram.rules_breaks.back()<<std::endl;

    }else{ //just copy the input
        std::ifstream in(i_file, std::ios_base::binary);
        std::ofstream out(o_file, std::ios_base::binary);

        auto buffer = reinterpret_cast<char*>(malloc(BUFFER_SIZE));
        do {
            in.read(&buffer[0], BUFFER_SIZE);
            out.write(&buffer[0], in.gcount());
            psize+=in.gcount();
        } while (in.gcount() > 0);
        free(buffer);
        psize/=sizeof(sym_type);

        //remove remaining files
        for(size_t i=0;i<threads_data.size();i++){
            std::string tmp_file =  threads_data[i].ofs.file;
            if(remove(tmp_file.c_str())){
                std::cout<<"Error trying to delete file "<<tmp_file<<std::endl;
            }
        }
    }
    return mp_table.size();
}

template void build_lc_gram<lms_parsing>(std::string &i_file, size_t n_threads, size_t hbuff_size, gram_info_t &p_gram,/*, alpha_t alphabet,*/ sdsl::cache_config &config);

template size_t build_lc_gram_int<lms_parsing<i_file_stream<uint8_t>, string_t>>(std::string &i_file, std::string &o_file, size_t n_threads, size_t hbuff_size,
                                           gram_info_t &p_gram, ivb_t &rules, bvb_t &rules_lim,
                                           /*sdsl::int_vector<2> &phrase_desc,*/ sdsl::cache_config &config);

template size_t build_lc_gram_int<lms_parsing<i_file_stream<size_t>, string_t>>(std::string &i_file, std::string &o_file, size_t n_threads, size_t hbuff_size,
                                          gram_info_t &p_gram, ivb_t &rules, bvb_t &rules_lim,
                                          /*sdsl::int_vector<2> &phrase_desc,*/ sdsl::cache_config &config);
