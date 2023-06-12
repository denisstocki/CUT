/*
    AUTHOR: DENIS STOCKI                  
    FILE: analyzer.h                       
    PURPOSE: interface for analyzer module 
*/

#ifndef ANALYZER_H
#define ANALYZER_H

// Usage of pseudo objective 
// encapsulation on struct analyzer.
typedef struct analyzer Analyzer;

Analyzer* Analyzer_init(void);
void Analyzer_free(Analyzer* analyzer);

#endif 
