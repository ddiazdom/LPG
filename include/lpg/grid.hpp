//
// Created by diego on 5/5/21.
//

#ifndef LPG_COMPRESSOR_GRID_H
#define LPG_COMPRESSOR_GRID_H


#include <sdsl/rrr_vector.hpp>
#include <sdsl/wt_int.hpp>
#include <sdsl/construct.hpp>

struct grid_point{
    size_t row;
    size_t col;
    size_t label;
    uint32_t level;
};

struct grid_query{
    size_t row1;
    size_t col1;
    size_t row2;
    size_t col2;
};
struct grid_query_results{
    size_t label;
    size_t col;
};

class grid {

public:


    typedef grid_point                                     point;
    typedef grid_query                                     query;
    typedef grid_query_results                          qresults;
    typedef size_t                                     size_type;
    typedef sdsl::wt_int<>                                  wt_s;
    typedef sdsl::rrr_vector<>                              bv_x;
    typedef sdsl::int_vector<>                                vi;

protected:
    wt_s sb;
    vi labels;

//    bv_x xa;
    bv_x xb;

//    bv_x::rank_1_type xb_rank1;
    bv_x::select_1_type xb_sel1;
//    bv_x::select_0_type xb_sel0;
//    bv_x::rank_1_type xa_rank1;

public:

    grid() = default;
    grid( const grid& _g ):sb(_g.sb), labels(_g.labels),xb(_g.xb) {
        compute_rank_select_st();
    }

    virtual ~grid() = default;

    void build(const std::vector<point>& _points,uint32_t level) {

        std::vector<point> level_points;
        size_type n_cols = 0,n_rows = 0, n_points = 0;
        for (const auto & _point : _points)
            if(_point.level == level){
                level_points.push_back(_point);
                /**
                 * compute max and min of col and row values...
                 * */
                n_cols = (n_cols < _point.col)? _point.col:n_cols;
                n_rows = (n_rows < _point.row)? _point.row:n_rows;
                ++n_points;
            }

        sort_points(level_points);
        build_bitvectors(level_points,n_cols,n_rows,n_points);
        build_wt_and_labels(level_points,n_cols,n_rows,n_points);
        compute_rank_select_st();
    }

protected:

    void sort_points(std::vector<point>& _points){
        /*
         * Sort _points by rows (rules) then by cols(suffix)
         * */
        sort(_points.begin(), _points.end(), [](const point &a, const point &b) -> bool
        {
            if (a.row < b.row) return true;
            if (a.row > b.row) return false;
            return (a.col) < (b.col);
        });
    }

    size_type map(const size_type  & row) const{
        assert(row > 0);
        return xb_sel1(row)-row+1;
    }

    void build_bitvectors(const std::vector<point>& _points, const size_type& n_cols,const size_type& n_rows,const size_type& n_points){

        std::vector<size_type> card_rows(n_rows, 0);
        std::vector<size_type> card_cols(n_cols, 0);

        /*
        * Computing the cardinal of every column and every row
        * */
        for (size_type i = 0; i < n_points; ++i) {
//            card_rows[_points[i].row - 1]++;
            card_cols[_points[i].col - 1]++;
        }

        /**
       * Building bit_vectors XA and XB
       *
       * */

        auto build_bv = [](const std::vector<size_type>& V,const size_type &n_points,const size_type &cardinality){
            sdsl::bit_vector X(n_points + cardinality + 1, 0);
            X[0] = 1;
            /**
             * Put a 0 in XB(XA) for each element in the ith row(col)
             * then add 1
             * */
            size_t p = 1;
            for (uint j = 0; j < cardinality; ++j) {
                X[p + V[j]] = true;
                p += V[j] + 1;
            }
            return X;
        };

        xb = bv_x(build_bv(card_cols,n_points,n_cols));
//        xa = bv_x(build_bv(card_rows,n_points,n_rows));
    }

    void build_wt_and_labels(const std::vector<point>& _points, const size_type& n_cols,const size_type& n_rows,const size_type& n_points){

        /**
         * Build a wavelet_tree on SB( index of the columns not empty ) and plain representation for SL(labels)
         * */
        std::ofstream sb_file("sb_file", std::ios::binary);
        sdsl::int_vector<> _sl(n_points,0);
        sdsl::int_vector<> _sb(n_points,0);

        size_type j = 0;
        for (size_type  i = 0; i < n_points; ++i) {
            _sl[j] = _points[i].label;
            _sb[j] = _points[i].col;
            ++j;
        }

        sdsl::util::bit_compress(_sl);
        sdsl::util::bit_compress(_sb);

        labels = vi(_sl);
        sdsl::serialize(_sb,sb_file);
        sb_file.close();
        //todo refactor wt construction...
        std::string id_sb = sdsl::util::basename("sb_file") + "_";
        sdsl::cache_config file_conf_sb(false,"./",id_sb);
        sdsl::construct(sb,"sb_file",file_conf_sb,0);

    }

    void compute_rank_select_st(){
        xb_sel1 = bv_x::select_1_type(&xb);
    }


public:

    void load(std::istream &in) {

        sdsl::load(labels,in);
        sdsl::load(sb,in);
        sdsl::load(xb,in);
        sdsl::load(xb_sel1,in);
    }

    size_type serialize(std::ostream &out, sdsl::structure_tree_node *v, std::string name) const {

//        sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        size_t written_bytes = 0;

        written_bytes += sdsl::serialize(labels,out);
        written_bytes += sdsl::serialize(sb ,out);
        written_bytes += sdsl::serialize(xb,out);
        written_bytes += sdsl::serialize(xb_sel1,out);

        return written_bytes;
    }

    size_type first_label_col(const size_type  & col) const{
        return labels[sb.select(1,col)];
    }

    void search_2d(const query& q,std::vector<size_type>& R){
        size_t p1,p2;
        p1 = map(q.row1);
        p2 = map(q.row2+1)-1;
        if(p1 > p2) return;
        auto res = sb.range_search_2d2(p1,p2,q.col1,q.col2);
        R.resize(res.first,0);qresults r;
        for ( size_type i = 0; i < R.size(); i++ ){
            R[i] = res.second[i].second;
        }
    }
};


class grid_t {

public:
    typedef grid                                            _grid;
    typedef size_t                                      size_type;
    typedef grid_point                                     point;
    typedef grid_query                                     query;
    typedef grid_query_results                          qresults;


protected:
    _grid *grid_levels{nullptr};
    uint32_t levels{};
public:

    grid_t () :grid_levels(nullptr),levels(0){}

    grid_t (const grid_t & _g ){
        assignMemory(_g.levels);
        for (uint32_t i = 0; i < levels ; ++i) {
            grid_levels[i] = _g.grid_levels[i];
        }
    }

    grid_t (const std::vector<point>& _points,const uint32_t &_l) {
        assignMemory(_l);
        for (uint32_t i = 0; i < levels ; ++i) {
            grid_levels[i].build(_points,i+1);
        }
    }

    virtual ~ grid_t() {
        safe_delete();
    }

    void load(std::istream &in) {
        sdsl::load(levels,in);
        safe_delete();
        assignMemory(levels);
        for (uint32_t i = 0; i < levels; ++i) {
            grid_levels[i].load(in);
        }
    }



    size_type serialize(std::ostream &out, sdsl::structure_tree_node *v, std::string name) const {
//        sdsl::structure_tree_node *child = sdsl::structure_tree::add_child(v, name, sdsl::util::class_name(*this));
        size_t written_bytes = 0;
        written_bytes += sdsl::serialize(levels,out);
        for (uint32_t i = 0; i < levels; ++i) {
            written_bytes += sdsl::serialize(grid_levels[i],out);
        }
        return written_bytes;
    }

protected:
    void safe_delete() {
        if(grid_levels!= nullptr)
            delete [] grid_levels;
    }
    void assignMemory(const uint32_t& _levels){
        levels = _levels;
        grid_levels = new _grid[levels];
    }
};

#endif //LPG_COMPRESSOR_GRID_H