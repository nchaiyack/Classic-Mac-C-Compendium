/*****************************************************************************
*   gbfeatdfn.h:
*   -- GenBank Feature table define file
*
* $Log: gbftdef.h,v $
 * Revision 1.2  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*                                                                  10-14-93
******************************************************************************/
#ifndef _GBFEATDFN_
#define _GBFEATDFN_


#define GBQUAL_anticodon          0
#define GBQUAL_bound_moiety       1
#define GBQUAL_cell_line          2
#define GBQUAL_cell_type          3
#define GBQUAL_chromosome         4
#define GBQUAL_chloroplast        5
#define GBQUAL_chromoplast        6
#define GBQUAL_citation           7
#define GBQUAL_clone              8
#define GBQUAL_clone_lib          9
#define GBQUAL_codon             10
#define GBQUAL_codon_start       11
#define GBQUAL_cons_splice       12
#define GBQUAL_cultivar          13
#define GBQUAL_cyanelle          14
#define GBQUAL_dev_stage         15
#define GBQUAL_direction         16
#define GBQUAL_EC_number         17
#define GBQUAL_evidence          18
#define GBQUAL_frequency         19
#define GBQUAL_function          20
#define GBQUAL_gene              21
#define GBQUAL_gdb_xref          22
#define GBQUAL_germline          23
#define GBQUAL_haplotype         24
#define GBQUAL_insertion_seq     25
#define GBQUAL_isolate           26
#define GBQUAL_kinetoplast       27
#define GBQUAL_label             28
#define GBQUAL_lab_host          29
#define GBQUAL_map               30
#define GBQUAL_macronuclear      31
#define GBQUAL_mitochondrion     32
#define GBQUAL_mod_base          33
#define GBQUAL_note              34
#define GBQUAL_number            35
#define GBQUAL_organism          36
#define GBQUAL_partial           37
#define GBQUAL_PCR_conditions    38
#define GBQUAL_pop_variant       39

#define GBQUAL_phenotype         40
#define GBQUAL_plasmid           41
#define GBQUAL_product           42
#define GBQUAL_proviral          43
#define GBQUAL_pseudo            44          
#define GBQUAL_rearranged        45
#define GBQUAL_rpt_family        46
#define GBQUAL_rpt_type          47
#define GBQUAL_rpt_unit          48
#define GBQUAL_sex               49
#define GBQUAL_sequenced_mol     50
#define GBQUAL_specific_host     51
#define GBQUAL_standard_name     52
#define GBQUAL_strain            53
#define GBQUAL_sub_clone         54
#define GBQUAL_sub_species       55
#define GBQUAL_sub_strain        56
#define GBQUAL_tissue_lib        57
#define GBQUAL_tissue_type       58
#define GBQUAL_translation       59
#define GBQUAL_transl_except     60
#define GBQUAL_transl_table      61
#define GBQUAL_transposon        62
#define GBQUAL_usedin            63
#define GBQUAL_variety           64

#define ParFlat_TOTAL_GBQUAL     65
#define ParFlat_TOTAL_IntOr       3
#define ParFlat_TOTAL_LRB         3
#define ParFlat_TOTAL_Exp         2
#define ParFlat_TOTAL_Rpt         7
#define ParFlat_TOTAL_GBFEAT     63

#define  Class_pos_aa             1
#define  Class_text               2
#define  Class_bracket_int        3
#define  Class_seq_aa             4
#define  Class_int_or             5
#define  Class_site               6
#define  Class_L_R_B              7
#define  Class_ecnum              8
#define  Class_exper              9
#define  Class_none              10
#define  Class_token             11
#define  Class_int               12
#define  Class_rpt               13
#define  Class_flabel_base       14
#define  Class_flabel_dbname     15
#define  Class_note              16


#define  ParFlat_Stoken_type            1
#define  ParFlat_BracketInt_type        2
#define  ParFlat_Integer_type           3      

/*********************************************************************/

typedef struct sematic_gbfeature {
    CharPtr  key;
    Int2     mand_num;
    Int2     mand_qual[5];
    Int2     opt_num;
    Int2     opt_qual[40];
} SematicFeat, PNTR SematicFeatPtr;



#endif
