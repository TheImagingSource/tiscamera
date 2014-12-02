
#ifndef ADJUST_GAIN_DB_TABLE_H_INC_
#define ADJUST_GAIN_DB_TABLE_H_INC_

// this calculates a gain adjustment based on the dist parameter
// precondition is that dist == 100 should be 'no adjustment', dist == 10 reduce by factor 10 and dist == 1000 increase by factor 10
int		adjust_gain_db( unsigned int dist );


#endif // ADJUST_GAIN_DB_TABLE_H_INC_