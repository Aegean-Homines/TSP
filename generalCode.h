//lower bound evaluation function
int
lower_bound( 
        Matrix const& m, 
        int    const& N, 
        int    const& index, 
        std::vector<int> &solution_so_far,
        int    const& cost_solution_so_far )
{
    int lower_bound = cost_solution_so_far;
    for ( int i=index; i<N; ++i ) {
        int min_in_row = std::numeric_limits<int>::max();
        for ( int j=0; j<N; ++j ) {
            //check if job is not assigned (column is taken)
            if ( std::find( solution_so_far.begin(), solution_so_far.end(), j) == solution_so_far.end() ) {
                if ( min_in_row > m[i][j] ) min_in_row = m[i][j];
            }
        }
        lower_bound += min_in_row;
    }
    return lower_bound;
}

std::vector<int>
backtracking_branch_bound_best_first( Matrix const& m, int const& N ) {
    std::vector<int> best_solution_so_far;
    std::vector<int> solution_so_far;
    int cost_solution_so_far = 0;
    int cost_best_solution_so_far = std::numeric_limits<int>::max();
    backtracking_branch_bound_best_first_aux( m, N, 0, solution_so_far, cost_solution_so_far, best_solution_so_far, cost_best_solution_so_far, 0 );
    return best_solution_so_far;
}

void
backtracking_branch_bound_best_first_aux( 
        Matrix const& m, 
        int    const& N, 
        int    const& index, 
        std::vector<int> &solution_so_far, 
        int& cost_solution_so_far, 
        std::vector<int> &best_solution_so_far, 
        int& cost_best_solution_so_far, 
        int parent_id ) 
{
    static int node_counter = 0;

    //termination check (if on last level)
    if ( index == N ) {
        std::cout << "solution " << cost_solution_so_far << " ( best " << cost_best_solution_so_far << " )" << std::endl;
        if ( cost_solution_so_far < cost_best_solution_so_far ) {
            cost_best_solution_so_far = cost_solution_so_far;
            best_solution_so_far = solution_so_far;
        }
    }

    //hack - use map to order, key - bound, value - job.
    //insert pairs, then traverse the map in-order
    std::multimap<int,int> ordered_jobs;

    //generate nodes and order them by bound
    for ( int j=0; j<N; ++j ) {
        //skip if job j is already assigned
        if ( std::find( solution_so_far.begin(), solution_so_far.end(), j) != solution_so_far.end() ) continue;
        solution_so_far.push_back( j );
        cost_solution_so_far += m[index][j];

        int lb = lower_bound( m, N, index+1, solution_so_far, cost_solution_so_far );
        ordered_jobs.insert ( std::pair<int,int>(lb,j) );
        solution_so_far.pop_back( );
        cost_solution_so_far -= m[index][j];
    }

    //traverse nodes in the above order - BEST FIRST
    std::multimap<int,int>::const_iterator b = ordered_jobs.begin(),
        e = ordered_jobs.end();
    for ( ; b != e; ++b ) {
        int j = b->second; // get job index;
        solution_so_far.push_back( j );
        cost_solution_so_far += m[index][j];

        int lb = b->first;
        //branch cancelation check
        //std::cout << "lower_bound = " << lower_bound( m, N, index, solution_so_far, cost_solution_so_far ) << std::endl;
        if ( lb < cost_best_solution_so_far ) {
            backtracking_branch_bound_best_first_aux( m, N, index+1, 
                    solution_so_far, cost_solution_so_far, 
                    best_solution_so_far, cost_best_solution_so_far, node_counter );
        }

        solution_so_far.pop_back( );
        cost_solution_so_far -= m[index][j];
    }
}

std::vector<int>
backtracking_branch_bound_best_first( Matrix const& m, int const& N ) {
    std::vector<int> best_solution_so_far;
    std::vector<int> solution_so_far;
    int cost_solution_so_far = 0;
    int cost_best_solution_so_far = std::numeric_limits<int>::max();
    backtracking_branch_bound_best_first_aux( m, N, 0, solution_so_far, cost_solution_so_far, best_solution_so_far, cost_best_solution_so_far, 0 );
    return best_solution_so_far;
}
