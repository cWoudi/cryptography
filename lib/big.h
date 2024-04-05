/*	bign.lib	v 1.05.03	April 2013	copyright Alan Firminger

	a library for large number mathematics
	described in buserdoc.htm */

/* to separate bign.h, copy out lines 7 to 660  8<...........................*/
/*	bign.h		*/

/* delete this comment and use these blank lines for any standing defines
   that you desire without changing the line numbers */








#ifndef BN_H_GOT
#define BN_H_GOT

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

void write_hex();
void read_hex();

#define PREC 64 
#define FRAC 0
#define WORD_SIZE 4
//#define NO_DEC
#define BN_VER 10503

		/*	settle BNLONG and L2(  )		*/
#if ( defined LLONG_MAX ) && ( ! defined NO_LL )
#if LLONG_MAX == 0x7FFFFFFFFFFFFFFFLL
#define BN_LL 1
#define L2( A ) A ## LL
#define BNLONG long long
#define DTYP 2
#define QLQ "ll"
#endif
#endif
#ifndef BN_LL
#define BN_LL 0
#define L2( A ) A ## L
#define BNLONG long
#define QLQ "l"
#endif
#define BNULONG unsigned BNLONG
#define BNELE unsigned int
			/*	int size definitions	*/
#if BN_LL || ( LONG_MAX == 0x7FFFFFFFFFFFFFFFL ) && ( INT_MAX == 0x7FFFFFFF )
#define E_SZ( A , B ) ( B )
#define BY_P_E 4
#define BNXPE L2( 8 )
#define QWQ "8"
#define BNFUL L2( 0xFFFFFFFFU )
#define BNSHIFT L2( 5 )
#define BNMASK L2( 31 )
#define B_P_E L2( 32 )
#ifndef NO_DEC
#define BNASZ( A ) ((L2(24394249U)*( A ))/L2(38539962U)+L2(9U)*( A )+L2(1U))
/* below, fraction is > log_10 ( 2 ) in 15th decimal place */
#define BNBSZ( A ) ((L2(7647217U)*( A +L2(1U)))/L2(25403505U)+L2(1U))
#define BNHOP 1003188697
#endif
#else
#if LONG_MAX == 0x7FFFFFFFL
#define E_SZ( A , B ) ( A )
#define BY_P_E 2
#define BNXPE 4L
#define QWQ "4"
#define BNFUL 0xFFFFUL
#define BNSHIFT 4L
#define BNMASK 15L
#define B_P_E 16L
#ifndef NO_DEC
#define BNASZ( A ) ((52031UL*( A ))/63726UL+4UL*( A )+1UL)
/* below, fraction is > * log_10 ( 2 ) in 9th decimal place */
#define BNBSZ( A ) ((4004UL*( A +1UL))/13301UL+1UL)
#define BNHOP 27208
#endif
#if INT_MAX == 0x7FFF
#else
#if SHRT_MAX == 0x7FFF
#undef BNELE
#define BNELE unsigned short
#define DTYP 1
#else
#error cannot compile with your compiler
#endif
#undef BN_LL
#endif

#else
#error cannot compile with your compiler
#endif
#endif
#ifndef DTYP
#define DTYP 0
#endif
			/*	default definitions	*/
#ifndef PREC
#define PREC 4
#endif
#ifndef FRAC
#define FRAC 0
#endif
#ifndef SIGNED
#define SIGNED 1
#endif

			/*	tests to stop compilation		*/
#if (PREC<4)||(FRAC<0)||(FRAC>=PREC)||(PREC>E_SZ(0x7FFB,0x7FFFFFFB))
#error stop in bign.h : PREC or FRAC are incompatible or out of range
#endif

#define LEN ( PREC + SIGNED )
#define S_LEN ( S_PREC + SIGNED )

		/*	type defines	*/
typedef BNELE big_n [ LEN ] ;
typedef BNELE bele ;
typedef BNLONG blong ;
typedef BNULONG ublong ;
#undef BNELE
#undef BNLONG
#undef BNULONG




			/*	consequent definitions	*/
#ifndef BIG_B_0
#define BIG_B_0 ( char ) '.'
#endif

#if FRAC
#ifndef BIG_B_POINT
#define BIG_B_POINT " . "
#endif
#else
#ifdef B_ROUND
#undef B_ROUND
#ifdef B_ROUND
#error B_ROUND defined with ! FRAC
#endif
#endif
#ifdef BIG_DAJ
#undef BIG_DAJ
#ifdef BIG_DAJ
#error BIG_DAJ defined with ! FRAC
#endif
#endif
#endif

#ifdef NO_DEC
#define BN_DEC( A )
#else
#define BN_DEC( A ) A
#endif

	/*	global variables or macro constants for soft size	*/
#ifndef NO_DEC
#if FRAC
#define FRAC_CHAR BNASZ( S_FRAC )
#endif
#define INT_CHAR BNASZ( S_PREC - S_FRAC )
#endif

#ifndef SOFT_SIZE
#define S_PREC PREC
#define s_prec PREC
#define S_FRAC FRAC
#define s_frac FRAC
#else
int s_prec = PREC ;
#define S_PREC s_prec
#define S_FRAC s_frac
#if FRAC
int s_frac = FRAC ;
#else
#define s_frac 0
#endif
#endif
			/*	coding definitions	*/
#if SIGNED
#define BN_SIG( A ) A
#define BN_NSIG( A )
#else
#define BN_SIG( A )
#define BN_NSIG( A ) A
#endif
#if FRAC
#define BN_FRAC( A ) A
#define BN_NFRAC( A )
#else
#define BN_FRAC( A )
#define BN_NFRAC( A ) A
#endif

#ifndef ERROR_MODE
#define ERROR_MODE 3
#endif

#if ( ERROR_MODE & 3 )
#define R_D int
#define BN_ERR3( A ) A
#define BN_RN return
#else
#define R_D void
#define BN_ERR3( A )
#define BN_RN
#endif

#if ERROR_MODE
#define BN_ERR( A ) A
#define BNEEOF EOF ==
#define BN_NERR && ! bnerr
#define BN_E int
#define BNE1 return ( 1 )
#define BNE0 return ( 0 )
#define BNDV
#else
#define BN_ERR( A )
#define BNEEOF
#define BN_NERR
#define BN_E void
#define BNE1 return
#define BNE0 return
#define BNDV return
#endif

#ifndef ERR_SM
#define ERR_SM stdout
#endif
#ifndef ERR_EX
#define ERR_EX ( - 1 )
#endif

#if ( ERROR_MODE & 4 )
#define BNE_FUN( sg ) error_function ( sg )
#define BN0
#elif ( ERROR_MODE & 2 )
#define BNE_FUN( sg ) { error_function ( sg ) ; return ( 1 ) ; }
#define BN0 return ( 0 )
#elif ( ERROR_MODE == 1 )
#define BNE_FUN( sg ) return ( 1 )
#define BN0 return ( 0 )
#elif ! ERROR_MODE
#define BNE_FUN( sg )
#define BN0
#endif

#ifdef USE_HEAP
#define BN_HEAP( A ) A
#define BN_NHEAP( A )
#define FREE_BN_( x ) free ( x )
#define DEF_BN_( x ) bele * x
#define BN_HEAP_( x ) x = ( bele * ) bn_h ( S_LEN , BY_P_E )
#else
#define BN_HEAP( A )
#define BN_NHEAP( A ) A
#define FREE_BN_( x )
#define DEF_BN_( x ) big_n x
#define BN_HEAP_( x )
#endif

#ifdef BIG_TRACE
#ifndef TR_SM
#define TR_SM stdout
#define BN_TRIN( A ) A ; fflush( stdout ) ;
#define BN_TRAC( A ) putc( A , stdout ) ; fflush( stdout ) ;
#else
#define BN_TRIN( A )
#define BN_TRAC( A ) putc( A , TR_SM ) ; 
#endif
#define BN_TROUT( A ) A ;
#define BN_NTR( A )
#else
#define BN_TRIN( A )
#define BN_TROUT( A )
#define BN_TRAC( A )
#define BN_NTR( A ) A
#endif

#ifndef FILE_SM
#define FILE_SM stdout
#endif

#if ( defined MDUF ) && ! FRAC
#undef MDUF
#ifdef MDUF
#error MDUF defined with ! FRAC
#endif
#endif

#ifdef MDUF
int muluf = MDUF , divuf = MDUF ;
#define BNCUF , divuf
#define BNUF( A ) A
#define BNCPABS( A , B ) bncp_abs ( A + nbot , B + nbot , bnact )
#else
#define muluf S_FRAC
#define divuf S_PREC
#define BNCUF
#define BNUF( A )
#define BNCPABS( A , B ) cp_abs ( A , B )
#endif

		/*	check and convert from ascii digits		*/
/*	bn_n_( c )	    c		yields
			  < 0           255
			 0 to 47	255
			48 to 57	x - 48
			58 to 64        255
			  > 64		x - 55			*/

#define bn_n_( c ) ((unsigned)(c-48)<10?c-48:c>64?c>96?c-87:c-55:255)

#ifndef B_ROUND
#define B_ROUND 0
#endif
		/*	i/o definitions		*/
#ifndef BIG_DAJ
#define BIG_DAJ ( FRAC > 0 )
#endif
#if ( BIG_DAJ == 1 ) && ( ! defined NO_DEC )
#define BNA_1 , 1
#define BNA_0 , 0
#else
#define BNA_1
#define BNA_0
#endif
#ifndef NAME_LEN
#define NAME_LEN 10
#endif
#ifndef BIG_MARK
#define BIG_MARK '#'
#endif
#ifndef BIG_END
#define BIG_END ','
#endif
#ifndef LINE_LEN
#define LINE_LEN 80
#endif
#ifndef CON_ROWS
#define CON_ROWS 25
#endif
#ifndef BIG_GROUP
#define BIG_GROUP 10
#endif
#ifndef X_x
#define X_x "0x "
#endif
#ifndef X_ten
#define X_ten 'A'
#endif

		/* read stream to big number */
#define sm_2_big_( sm , x ) big_in ( sm , BIG_MARK , x , 10 , BIG_END )
#define smx_2_big_( sm , x ) big_in ( sm , BIG_MARK , x , 16 , BIG_END )

#define BNXICH ( ( blong ) ( ( S_PREC - S_FRAC ) * BNXPE ) )

		/* V  chip  mode   chfp  leadz base   V */
#ifndef NO_DEC
#ifndef OUT_DEF
#if FRAC
#define OUT_DEF     L2( 0 ) , 0 , L2( 0 ) , ' ' , 10
#else
#define OUT_DEF     L2( 0 ) ,               ' ' , 10
#endif
#endif
#endif
#ifndef HEX_DEF
#if FRAC
#define HEX_DEF     BNXICH , 0 , L2( 0 ) , '0' , 0
#else
#define HEX_DEF     BNXICH ,               '0' , 0
#endif
#endif
		/*   mark       end       group      line */
#ifndef BOD_DEF
#define BOD_DEF      ' ' ,       0    , BIG_GROUP , LINE_LEN
#endif
#ifndef MC_DEF
#define MC_DEF     BIG_MARK , BIG_END , BIG_GROUP , LINE_LEN
#endif

		/*	convenient i/o defines		*/
/* x decimal to screen */
#define cn_( x ) big_out ( 0 , NULL , x , stdout , BOD_DEF , OUT_DEF )
/* x decimal to printer */
#define pr_( x ) big_out ( 0 , NULL , x , stdprn , BOD_DEF , OUT_DEF )
/* x decimal to stream for humans */
#define ob_( x , sm ) big_out ( 0 , NULL , x , sm , BOD_DEF , OUT_DEF )
/* x decimal to stream for m/c */
#define om_( x , sm ) big_out ( 0 , NULL , x , sm , MC_DEF , OUT_DEF )

/* string then x decimal to screen */
#define c2_( sg , x ) big_out ( NAME_LEN , sg , x , stdout , BOD_DEF , OUT_DEF )
/* string then x decimal to printer */
#define p2_( sg , x ) big_out ( NAME_LEN , sg , x , stdprn , BOD_DEF , OUT_DEF )
/* string then x decimal to stream for humans */
#define ob2_( sg , x , sm ) big_out ( NAME_LEN, sg, x, sm, BOD_DEF, OUT_DEF )
/* string then x decimal to stream for m/c */
#define om2_( sg , x , sm ) big_out ( NAME_LEN, sg, x, sm, MC_DEF, OUT_DEF )

/* x hex to screen */
#define cx_( x ) big_out ( 0 , NULL , x , stdout , BOD_DEF , HEX_DEF )
/* x hex to printer */
#define px_( x ) big_out ( 0 , NULL , x , stdprn , BOD_DEF , HEX_DEF )
/* x hex to stream for humans */
#define obx_( x , sm ) big_out ( 0 , NULL , x , sm , BOD_DEF , HEX_DEF )
/* x hex to stream for m/c */
#define omx_( x , sm ) big_out ( 0 , NULL , x , sm , MC_DEF , HEX_DEF )
/* string then x hex to screen */
#define cx2_( sg , x ) big_out ( NAME_LEN, sg, x, stdout, BOD_DEF, HEX_DEF )
/* string then x hex to printer */
#define px2_( sg , x ) big_out ( NAME_LEN, sg, x, stdprn, BOD_DEF, HEX_DEF )
/* string then x hex to stream for humans */
#define obx2_( sg , x , sm ) big_out ( NAME_LEN, sg, x, sm, BOD_DEF, HEX_DEF)
/* string then x hex to stream for m/c */
#define omx2_( sg , x , sm ) big_out ( NAME_LEN, sg, x, sm, MC_DEF, HEX_DEF )
#define tr_pr_( A , B ) big_out ( 5 , A , B , TR_SM , MC_DEF , HEX_DEF )

#ifndef USE_HEAP
#ifndef NO_DEC
#define BN_NEED ( BNASZ( PREC ) + SIGNED + ( FRAC > 0 ) * 2 )
#else
#define BN_NEED ( PREC * BY_P_E * 2 + SIGNED + ( FRAC > 0 ) + 2 )
#endif
#else
#ifndef NO_DEC
#define BN_NEED ( SIGNED + BNASZ( S_PREC ) + ( FRAC > 0 ) * 2 )
#else
#define BN_NEED ( S_PREC * BY_P_E * 2 + SIGNED + ( FRAC > 0 ) + 2 )
#endif
#endif

#ifndef BNKB_MAX
#ifdef GET_C_POS
#define BNKB_MAX ( LINE_LEN * 5 )
#else
#define BNKB_MAX ( 80 )
#endif
#endif

#define BNKB_CH ( BN_NEED > BNKB_MAX ? BNKB_MAX : BN_NEED )

	/*	structures for decimal conversion control	*/
#ifndef NO_DEC
struct bn_in { bele * x ; BN_FRAC( bele * h ; )
		ublong ulbuff ; blong n ; int m ; } ;
#endif
struct bnout { int nam_len ; char * sg ; int mark ; bele * x ; int b_end ;
	blong group , line_len ; int base ; FILE * sm ;
	blong n ; blong chip , achip ;
	BN_FRAC( int ofm ; int frnd ; int fpnl ; blong chfp ; )
	int leadz , lead1 ;
#ifndef NO_DEC
	bele * n00ip ;
#if ( defined USE_HEAP && defined SOFT_SIZE ) || ( PREC > BNHOP )
	bele * n00top ; bele * n002 ;
#endif
#endif
	bele bebuff ; int ( * fcall ) ( struct bnout * ctrl ) ; } ;

		/*	function prototypes		*/
/*	utilities	*/
void zero_big ( bele * w ) ;
void bnflen ( bele * w , unsigned sz , bele u ) ;
void copy_big ( bele * w , bele * x ) ;
void bnclen ( bele * w , bele * x , unsigned m ) ;

/*	maths, big on big	*/
R_D add_big ( bele * w , bele * x , bele * y ) ;
#if SIGNED
BN_E bnaddub ( bele * w , bele * x , bele * y ) ;
#else
#define bnaddub rawadd_big
#endif
R_D sub_big ( bele * w , bele * x , bele * y ) ;
void bns_ub ( bele * w , bele * x , bele * y ) ;
R_D mult_big ( bele * w , bele * x , bele * y ) ;
R_D div_big ( bele * w , bele * x , bele * y ) ;
R_D mod_big ( bele * w , bele * x , bele * z ) ;
R_D div_big_4 ( bele * w , bele * x , bele * y , bele * z ) ;
#if FRAC
#if B_ROUND
#define BN_ROUND | INT_MIN
#else
#define BN_ROUND
#endif
#ifdef MDUF
#define rawdiv_big_( W , X , Y ) bn_divide ( W,X,Y , NULL , 0 BN_ROUND , divuf )
#define bndo0_( W , X , Y ) bn_divide ( W , X , Y , NULL , 0 , divuf )
#define rawmod_big_( W , X , Y ) bn_divide( W , X , NULL , Y , S_FRAC , S_PREC )
#define rawi_div_( W , X , Y ) bn_divide ( W , X , Y , NULL , S_FRAC , S_PREC )
#define rawdiv_4_( W , X , Y , Z ) bn_divide ( W , X , Y , Z , S_FRAC , S_PREC )
#else
#define rawdiv_big_( W , X , Y ) bn_divide ( W , X , Y , NULL , 0 BN_ROUND )
#define bndo0_( W , X , Y ) bn_divide ( W , X , Y , NULL , 0 )
#define rawmod_big_( W , X , Z ) bn_divide ( W , X , NULL , Z , S_FRAC )
#define rawi_div_( W , X , Y ) bn_divide ( W , X , Y , NULL , S_FRAC )
#define rawdiv_4_( W , X , Y , Z ) bn_divide ( W , X , Y , Z , S_FRAC )
#endif

BN_E bn_divide ( bele * w , bele * x , bele * y , bele * z , int iend
#ifdef MDUF
	, int bnuf
#endif
	) ;
R_D i_div_big ( bele * w , bele * x , bele * y ) ;
#else
#define rawdiv_big_( W , X , Y ) bn_divide ( W , X , Y , NULL )
	  //#define rawmod_big_( W , X , Y ) rawdiv_big_( W , X , Y )
#define rawmod_big_( W , X , Y ) bn_divide ( W , X , NULL,Y )
#define rawi_div_( W , X , Y ) bn_divide ( W , X , Y , NULL )
#define rawdiv_4_( W , X , Y , Z ) bn_divide ( W , X , Y , Z )
#define bndo0_( W , X , Y ) bn_divide ( W , X , Y , NULL )
BN_E bn_divide ( bele * w , bele * x , bele * y , bele * z ) ;
#define i_div_big div_big
#endif

#if ( ERROR_MODE > 1 ) || ( defined BIG_TRACE )
BN_E rawadd_big ( bele * w , bele * x , bele * y ) ;
BN_E rawsub_big ( bele * w , bele * x , bele * y ) ;
BN_E rawmult_big ( bele * w , bele * x , bele * y ) ;
#else
#define rawadd_big add_big
#define rawsub_big sub_big
#define rawmult_big mult_big
#endif

R_D big_shift ( blong shift , bele * x , bele * y ) ;
void bns_up ( ublong shift , bele * x , bele * y , int m ) ;
void bns_down ( ublong shift , bele * x , bele * y , int m ) ;

/*	maths, big on int	*/
R_D div_bintoi ( bele * x , int i , bele * y ) ;
#if ( ERROR_MODE > 1 ) || ( defined BIG_TRACE )
BN_E rawdivbintoi ( bele * x , int i , bele * y ) ;
#else
#define rawdivbintoi div_bintoi
#endif

/*	maths, int value on big	*/
R_D add_int ( int i , bele * x , bele * y ) ;
R_D sub_int ( int i , bele * x , bele * y ) ;
BN_E add_li ( blong li , bele * x ) ;
BN_E add_li_place ( blong li , bele * x , int m ) ;
int bnadd_ul ( ublong ul , bele * x , int m ) ;
void bnsub_ul ( ublong ul , bele * x , int m ) ;
void bns_lipos ( ublong li , bele * x , int m ) ;
R_D mult_int ( int i , bele * x , bele * y ) ;
BN_E mult_li ( blong li , bele * x , bele * y ) ;
unsigned bnmulul ( ublong ul , bele * x , bele * y , int sz ) ;
R_D div_int ( int i , bele * x , bele * y ) ;
#if ! ( FRAC )
R_D div_i_4 ( int i , bele * x , bele * y , int * rem ) ;
BN_E div_li_4 ( blong li , bele * x , bele * y , blong * rem ) ;
#endif
BN_E div_li ( blong li , bele * x , bele * y ) ;
unsigned bndivul ( ublong lucy , ublong li , bele * x , bele * y , int m ) ;

/*	number testing	*/
blong overbit ( bele * x ) ;
blong a_obitdif ( bele * x , bele * y ) ;
blong obitdif ( bele * x , bele * y ) ;
#if SIGNED
blong bnbn_uobit ( bele * cp_absx ) ;
blong bn_ubdif ( bele * x , bele * y ) ;
blong bn_ubsum ( bele * x , bele * y ) ;
#else
#define bn_ubdif obitdif
#endif
blong mssb ( bele * x ) ;
blong bn_msx ( bele * x , int sz ) ;
#if FRAC
blong ntop ( bele * x ) ;
#else
#define ntop mssb
#endif
int cp_abs ( bele * x , bele * y ) ;
#ifdef MDUF
int bncp_abs ( bele * w , bele * x , int m ) ;
#else
#define bncp_abs cp_abs
#endif

/*	conversion, input & output and auxiliaries	*/
R_D big_in ( FILE * sm , int mark , bele * x , int base , int b_end ) ;
#if ( ERROR_MODE > 1 ) || ( FRAC && SIGNED )
BN_E bnin1 ( FILE * sm , int mark , bele * x , int base ,
	int b_end
#if ( FRAC && SIGNED )
	, int xtra
#endif
	) ;
#else
#define bnin1 big_in
#endif
int bnin2 ( FILE * sm , bele * z , int base , int ich , int b_end
#if ( FRAC && SIGNED )
	, int xtra
#endif
#ifndef NO_DEC
		, struct bn_in * sc ) ;
int bn_dec_in ( FILE * sm , bele * x , int ich , int b_end
#endif
		) ;

int bn2i ( FILE * sm , int i_end ) ;
void bnplant ( unsigned ich , bele * x , blong i ) ;
#ifndef NO_DEC
int bn_dec_in_ip ( int ich , struct bn_in * sc ) ;
int bn_decip_end ( struct bn_in * sc ) ;
BN_FRAC( void bn_dec_in_fp ( int ich , struct bn_in * sc ) ; )
BN_FRAC( void bn_decfp_end ( struct bn_in * sc ) ; )
#endif

int kb_2_big ( bele * x ) ;
int kb_2_big_2 ( bele * x , int base ) ;

R_D num_here ( char * sg , bele * x ) ;
int bnnum_here ( char * sg , int base , bele * x
#if ( BIG_DAJ == 1 ) && ( ! defined NO_DEC )
	, int asc_inc
#endif
			) ;




R_D big_out ( int name_len , char * sg , bele * x , FILE * sm ,
	int mark , int b_end , int group , unsigned line_len , blong chip ,
#if FRAC
		int fmode , blong chfp ,
#endif
		int leadz , int base ) ;
BN_E bn_readyx ( struct bnout * ctrl ) ;
#ifndef NO_DEC
BN_E bn_m_00 ( struct bnout * ) ;
#endif
BN_E bn_s_out ( struct bnout * ) ;
#ifndef NO_DEC
void bni10i ( struct bnout * ) ;
int bn210 ( struct bnout * ) ;
#endif
int bn2x ( struct bnout * ) ;

void bit_show ( bele * w ) ;
R_D bits_out ( bele * w , int m , FILE * sm ) ;
BN_E bn_bits2sm ( bele be , FILE * sm ) ;

/*	error function	*/
#if ( ( ERROR_MODE & 63 ) > 1 )
void error_function ( char * sg ) ;
#endif
#ifdef USE_HEAP
void * bn_h ( int i , int j ) ;
#endif

#endif

