#include "big.h"
#include <string.h>

#if ! ( defined BN_LIB_GOT || defined NO_BIG_BODY )
#define BN_LIB_GOT


void read_hex(FILE* stream,bele* x)
{
	char buffer[4096];
	fgets(buffer, sizeof buffer, stream);	
	buffer[strcspn(buffer, "\n")] = 0;
	num_here(buffer,x);
}


void write_hex(FILE* stream,bele* x,int cend)
{
	blong limit = mssb(x);
	blong nb_words = limit / B_P_E;

	if (x[PREC] == 1)
		fprintf(stream,"-");

	fprintf(stream,"0x%0X",x[nb_words]);
	for (int i = nb_words - 1; i >= 0; i--)
		fprintf(stream,"%08X",x[i]);
	fprintf(stream,"%c",cend);
}

/*	physical		*/

void zero_big ( bele * w )					/* set w = 0 */
{ BN_TRAC( '0' ) ; bnflen ( w , S_LEN , ( bele ) 0 ) ; }

void bnflen ( bele * w , unsigned sz , bele u )
{ while ( sz -- ) w [ sz ] = u ; }

void copy_big ( bele * w , bele * x )
{ BN_TRAC( 'c' ) ; bnclen ( w , x , S_LEN ) ; }

void bnclen ( bele * w , bele * x , unsigned m )
{ while ( m -- ) x [ m ] = w [ m ] ; }

/*	big on big mathematics		*/

/**	  add_big      sub_big
  |		  |
  raw_add_big   raw_sub_big
  | \/ |			depending on signs & add or sub
  | /\ |
  bnaddub  bns_ub		*/

R_D add_big ( bele * w , bele * x , bele * y )	/* y = w + x */
#if ( ERROR_MODE > 1 ) || ( defined BIG_TRACE )
{ BN_TRAC( '+' ) ;
	BN_ERR( if ) ( rawadd_big ( w , x , y ) ) BNE_FUN( "overflow in add_big" ) ;
	BN0 ; }

BN_E rawadd_big ( bele * w , bele * x , bele * y )
#endif
#if SIGNED
{ if ( w [ S_PREC ] != x [ S_PREC ] )
	{ ( bns_ub ( w , x , y ) ) ; BN_ERR( return ( 0 ) ) ; }
	else
		BN_ERR( return ) ( bnaddub ( w , x , y ) ) ; }	/* returns 1 or 0 */

BN_E bnaddub ( bele * w , bele * x , bele * y )	/* y = w + x */
#endif
{ ublong ul = L2( 0 ) ;
	int m = 0 ;
	while ( m < S_PREC )
	{ y [ m ] = ( bele )
		( ul = ( ublong ) ( ul > BNFUL ) + w [ m ] + x [ m ] ) ;
		++ m ;
		BN_SIG( y [ S_PREC ] = x [ S_PREC ] ; ) }
	BN_ERR( return ( ( int ) ( ul >> B_P_E ) ) ; ) }	     /* returns carry */

R_D sub_big ( bele * w , bele * x , bele * y )	/* y = x - w */
#if ( ERROR_MODE > 1 ) || ( defined BIG_TRACE )
{ BN_TRAC( '-' ) ;
	BN_ERR( if ) ( rawsub_big ( w , x , y ) ) BNE_FUN( "overflow in sub_big" ) ;
	BN0 ; }

BN_E rawsub_big ( bele * w , bele * x , bele * y )
#endif
{
#if SIGNED
	if ( w [ S_PREC ] != x [ S_PREC ] )
		BN_ERR( return ) ( bnaddub ( w , x , y ) ) ;
	else
#endif
		bns_ub ( w , x , y ) ;
	BN_ERR( return ( 0 ) ) ; }

void bns_ub ( bele * w , bele * x , bele * y )
{ blong li = L2( 0 ) ; bele * e_ptr ;
	int m = 0 ;
	if ( cp_abs ( w , x ) == - 1 )			/* abs w > abs x */
	{ BN_SIG( y [ S_PREC ] = x [ S_PREC ] ^ 1 ; )		/* carry through zero */
		e_ptr = w ; w = x ; x = e_ptr ; }			/* swap pointers */
	BN_SIG( else y [ S_PREC ] = x [ S_PREC ] ; )		/* no change */
		while ( m < S_PREC )
			y [ m ++ ] = ( bele ) ( li = ( blong ) * x ++ - * w ++ - ( li < 0 ) ) ; }

R_D mult_big ( bele * w , bele * x , bele * y )	/* y = x * w */
#if ( ERROR_MODE > 1 ) || ( defined BIG_TRACE )
{ BN_TRAC( '*' ) ;
	BN_ERR( if ) ( rawmult_big ( w , x , y ) )
		BNE_FUN( "overflow in mult_big" ) ;
	BN0 ; }

	/** multiply two big numbers w * x to y
	  w [ m ] * x [ d + m - S_FRAC ] accumulates into y [ d ] and a carry
	  the outer loop increments each element of the destination in turn
	  starting at w [ 0 ] * x [ 0 ] to provide y [ - S_FRAC ]
	  during each sweep the multiple is accumulated in two ublong
	  if the destination has an index less than 0 then the answer is discarded
	  the carry always starts the next run
	  the inner loops are an oblique sweep, like these :

	  outer loop starts at w [ 0 ]
	  each sweep start at x [ 0 ]
	  v
	  x [  ]   0 1 2 3 4 

	  w [ 3 ]    o o o o o   > y [i]	   i = highest element affected
	  \ \ \ \
	  w [ 2 ]    o o o o o   > y [h]
	  \ \ \ \
	  w [ 1 ]    o o o o o   > y [g]
	  \ \ \ \
	  w [ 0 ]    o o o o o   > y [f]
	  v v v v v 
	  to   y [  ]   a b c d e
	  a = - S_FRAC and increments to b, c, ...		*/


BN_E rawmult_big ( bele * w , bele * x , bele * y )
#endif
{ ublong ul , lucy ;
	int tw , tx , ty , sw = 0 , sx = 0 , iw , ix , iy ;
	/* ^    tops	        starts		indices */
	bele * swap ;
#if B_ROUND > 1
	unsigned underrun = 0 , up ;
#endif
	DEF_BN_( z ) ;
	tw = ( int ) ( blong ) mssb ( w ) >> BNSHIFT ;
	tx = ( int ) ( blong ) mssb ( x ) >> BNSHIFT ;
	if ( ( ty = tw + tx BN_FRAC( - S_FRAC ) ) >= S_PREC ) BNE1 ;
	if ( tw > tx )
	{ swap = w ; w = x ; x = swap ; iw = tw ; tw = tx ; tx = iw ; }
	BN_HEAP_( z ) ;
	BN_NHEAP( bnflen ( z , S_LEN , ( bele ) 0 ) ; )
		BN_SIG( z [ S_PREC ] = w [ S_PREC ] ^ x [ S_PREC ] ; )
		ul = lucy = L2( 0 ) ;					/* initialise */
	iy = ( BNUF( ( unsigned ) muluf < S_FRAC ? - muluf : ) - S_FRAC - 1 ) ;
	BNUF( sw = S_FRAC + iy + 1 ; )
		/*	outer loop	*/
		while ( iy ++ < ty )
		{ iw = sw ; ix = sx ;				 	/* active starts */
			/*	inner loop	*/
			do
			{ ul += ( ublong ) w [ iw ] * x [ ix ++ ] ;	  	/* accumulate * */
				lucy += ( ul >> B_P_E ) ;				/* accumulate carry */
				ul = ( bele ) ul ; }				/* no hi carry in ul */
			while ( iw -- ) ;					/* while effective */
			/* ^	end inner loop	^ */
#if B_ROUND > 1
			if ( iy < - 1 ) underrun = underrun | ul ;
			if ( iy == - 1 )
			{ underrun = underrun | ( ul & E_SZ( 0x7FFF , 0x7FFFFFFF ) ) ;
				up = ul & E_SZ( 0x8000UL , L2( 0x80000000U ) ) ; }
			if ( ( ! iy ) && up && ( underrun || ( ( ul ^ B_ROUND ) & 1 ) ) )
				if ( ! ( bele ) ++ ul ) ++ lucy ;		/* round up, & carry */
#elif B_ROUND == 1
			if ( ( iy == - 1 ) && ( ul & E_SZ( 0x8000UL , L2( 0x80000000U ) ) ) )
				++ lucy ;
#endif
			BN_FRAC( if ( iy > - 1 ) )				/* if valid place */
				z [ iy ] = ( bele ) ul ;			/* take accumulation */
			ul = lucy & E_SZ( 0xFFFF , 0xFFFFFFFF ) ;		/* low carry for next */
			lucy = lucy >> B_P_E ;				/* high to low carry */
			if ( sw < tw ) ++ sw ;
			else
			{ ++ sx ;
				if ( ix > tx )
				{ -- sw ; -- tw ; ++ w ; -- tx ; } } }		/* ugly , fast */
	/* ^	end outer loop	^ */
	if ( iy < S_PREC ) z [ iy ] = ( bele ) ul ;
	bnclen ( z , y , S_LEN ) ;
	FREE_BN_( z ) ;
	BN_ERR( return ( ( int ) ( ( blong ) ( ( int ) iy == S_PREC ) && ul ) ) ) ; }

	/** there is one engine bn_divide
	  these functions for a user pass to it :
	  div_big , mod_big , i_div_big & div_big_4
	  and these definitions, in the header, call to it :
	  rawdiv_big_ , bndo0_ , rawmod_big_ , rawi_div_ & rawdiv_4_

	  definitions for the divide below	*/

#if FRAC
#define BNZ_0 , 0
#ifndef SOFT_SIZE
#define BNZ_F , s_frac
#else
#define BNZ_F , 1
#endif
#else
#define BNZ_0
#define BNZ_F
#define bnend 0
#endif

R_D div_big ( bele * w , bele * x , bele * y )		/* y = w / x */
{ BN_TRAC( '/' ) ;
	BN_ERR( if ) ( rawdiv_big_( w , x , y ) )
		BNE_FUN( "overflow or divide by zero in div_big" ) ;
	BN0 ; }

R_D mod_big ( bele * w , bele * x , bele * z )		/* z = x % w */
{ BN_TRAC( '%' ) ;
	BN_ERR( if ) ( rawmod_big_( w , x , z ) )
		BNE_FUN( "divide by zero in mod_big" ) ;
	BN0 ; }

#if FRAC
R_D i_div_big ( bele * w , bele * x , bele * y )	/* y = w \ x */
{ BN_TRAC( '\\' ) ;
	BN_ERR( if ) ( rawi_div_( w , x , y ) )
		BNE_FUN( "overflow or divide by zero in i_div" ) ;
	BN0 ; }
#endif

R_D div_big_4 ( bele * w , bele * x , bele * y , bele * z )
{ BN_TRAC( '4' ) ;
	BN_ERR( if ) ( rawdiv_4_( w , x , y , z ) )
		BNE_FUN( "overflow or divide by zero in div_big_4" ) ;
	BN0 ; }

#undef BNZ_0
#undef BNZ_F

	/** set the residue to the dividend
	  bn_divide takes the active ( ( top bele length of the divisor ) + 1 )
	  the outer loop comprises
	  new top answer is =
	  ( active residue top ) divided by ( divisor top + 1 )	    (1)
	  shift up the residue to the top or ( top - 1 ) bit
	  add new top answer into the answer at the correct bit place
	  inner loop runs up the numbers :
	  residue = residue - ( top answer * dividend )
	  if the top answer was 1 short then this is corrected by the next loop
	  and finally test if the answer is 1 short and fix
	  round

	  (1) division of the ( top of divisor + 1 ) into the residue top ublong length
	  consider a ublong as two bele here : bele,bele , and ... continues the same
	  the top answer ranges from 0,80...0 to 1,F...FC, ratio of nearly 1 to 4
	  and if the top multiplier is greater than 0,F...F
	  then the residue shift is to one bit less than the top

	  bit places		w_p	divider, then divider shift
	  y_p	dividend
	  z_p	residue

	  shifts		z_s	residue
	  z_st	accumulation of shift for answer shift

	  top two elements	w_t	divider , after assignment does not change
	  z_t	local residue		*/

BN_E bn_divide ( bele * w , bele * x , bele * y , bele * z
#if FRAC
		, int iend
#endif
#ifdef MDUF
		, int bnuf
#endif
		)
{ DEF_BN_( w2 ) ;
	blong w_p , y_p , z_p , z_s , z_st = - B_P_E ;   /* bit places and shift */
	ublong w_t , z_t , lj , lcy ;
	/* bit positions are	w_p	divider, then divider shift
	   y_p	dividend
	   z_p	residue
	   shifts		z_s	residue
	   z_st	accumulation of shift for answer shift
	   top two elements	w_t	divider , after assignment does not change
	   z_t	local residue		*/
	int k , m , n ;
	bele * z1 ;
#if FRAC
	int bnend ;
#if B_ROUND
	int j ;
#endif
	blong bnlend ;
#else
#define bnend 0
#define bnlend L2( 0 )
#endif

#ifdef MDUF
	int mv , bnact , nbot = 0 ;	/* move , active length , shift place */
	bele * bnzx ;
#else
#define bnuf S_PREC
#define bnact S_PREC
#define bnzx zx
#endif
#ifndef USE_HEAP
	bele zx [ PREC + 1 ] ;			/* 4 args is only ! FRAC */
#else
	bele * zx ;
#endif
#if FRAC
	bnend = iend & INT_MAX ;
#endif
	/*	set y = NULL if no division required		*/
	if ( y == z ) y = NULL ;				/* no division */
	/*	initialise top bit controls , divisor and residue	*/
	if ( ( w_p = bn_msx ( w , S_PREC ) ) == - L2( 1 ) )
		BNE1 ;						/* return if / 0 */
	if ( ( z_p = bn_msx ( x , S_PREC ) ) == - L2( 1 ) )	/* divide into 0 */
	{ if ( y != NULL ) bnflen ( y , S_LEN , 0 ) ;	/* returns to 0 */
		if ( z != NULL ) bnflen ( z , S_LEN , 0 ) ;
		BNE0 ; }						/* return */
	y_p = z_p - w_p
		BN_FRAC( + ( ( blong ) S_FRAC << BNSHIFT ) ) ;	/* top answer place */
	z_s = ( ( blong ) S_PREC + L2( 1 ) ) * B_P_E - L2( 1 ) - z_p ;
	/* first zx shift */
#ifdef USE_HEAP
	/*	take w2 , zx [ LEN + 1 ] and set * z1	 	*/
	BN_HEAP_( w2 ) ;
	zx = ( bele * ) bn_h ( S_PREC + 1 , BY_P_E ) ;
#endif
	z1 = & zx [ 1 ] ;				/* for 1 u-f int */
#ifdef MDUF
	bnuf = ( unsigned ) bnuf > S_PREC ? S_PREC : bnuf ;
	bnact = S_PREC ;
	bnzx = zx ;
#endif
	/*	prepare registers	 	*/
	BN_FRAC( bnlend = ( blong ) bnend << BNSHIFT ; )
		k = ( int ) ( ( blong ) ( w_p =
					( ( blong ) S_PREC << BNSHIFT ) - L2( 1 ) - w_p ) >> BNSHIFT ) ;
	bns_up ( w_p , w , w2 , S_PREC ) ;		/* divisor to top of w2 */
	bnclen ( x , zx , S_PREC ) ;		/* dividend to residue low */
	zx [ S_PREC ] = 0 ;				/* extra to be empty */
	if ( y != NULL )
		bnflen ( y , S_PREC , 0 ) ;			/* to collect answer */
	/*	take divisor top + 1		*/
	w_t = ( ublong ) w2 [ S_PREC - 1 ] + L2( 1 ) ;
	/*	outer loop		*/
	while ( 1 )
	{	/*	bit and element top zx places		*/
		lj = z_p & BNMASK ;					/* mod top bit place */
		m = ( int ) ( ( ublong ) z_p >> BNSHIFT ) ;		/* zx element place */
		/*	take residue top and divide into		*/
		z_t = ( ublong ) ( ( ( ublong ) zx [ m ] ) << ( B_P_E + BNMASK - lj ) ) |
			( ( m > 0 ) ? ( ublong ) zx [ m - 1 ] << ( BNMASK - lj ) : L2( 0 ) ) |
			( ( m > 1 ) ? ( ublong ) zx [ m - 2 ] >> ( L2( 1 ) + lj ) : L2( 0 ) ) ;
		if ( ( z_t /= w_t ) > BNFUL )			/* top divide */
		{ z_t >>= L2( 1 ) ; -- z_s ; ++ y_p ; }		/* if larger */
		z_st += z_s ;
		/*	shift zx to top			*/
		bns_up ( z_s , bnzx , bnzx , bnact + 1 ) ;		/* residue to top */
		/*	test if done			*/
		if ( y_p <= bnlend ) break ;
		/*	MDUF business			*/
#ifdef MDUF
		if ( ( ( unsigned ) bnuf < S_PREC ) &&
				( ( mv = bnact - ( y_p >> BNSHIFT ) - 1 - bnuf ) > 0 ) )
		{ bnact -= mv ; bnzx += mv ; nbot += mv ; }
#endif
		/*	set m to top answer element place, curtail and add	*/
		m = y_p >> BNSHIFT ; lj = y_p & BNMASK ;
		if ( m == bnend ) z_t &= ( BNFUL << ( B_P_E - lj ) ) ;	/* mask ans */
		/*	add top answer at bit place, ? o-f		*/
		if ( y != NULL )					/* if collecting */
		{ BN_FRAC( if )					/* o-f only if FRAC */
			( bnadd_ul
			  ( z_t << ( y_p & BNMASK ) , y , m - 1 ) )	/* m cont */
				BN_FRAC( { FREE_BN_( zx ) ; FREE_BN_( w2 ) ; BNE1 ; } )
				; }						/* return if o-f */
		/*	run up zx subtracting multiple		*/
		BNUF( if ( k < nbot ) k = nbot ; )
			n = k ;
		lcy = L2( 0 ) ;					/* initialise run */
		do
		{ lcy += z_t * ( ublong ) w2 [ n ] ;
			if ( ( bele ) lcy > zx [ n ] )
				lcy += E_SZ( 0x10000UL , L2( 0x100000000U ) ) ;
			zx [ n ] -= ( bele ) lcy ; lcy >>= B_P_E ;
		} while ( ++ n < S_PREC ) ;
		zx [ S_PREC ] -= ( bele ) lcy ;			/* top element */
		/*	set new top bit places and zx shift		*/
		z_s = ( ( ( blong ) S_PREC + L2( 1 ) ) * B_P_E - L2( 1 ) ) -
			( z_p = bn_msx ( zx , S_PREC + 1 ) ) ;		/* residue top place */
#ifdef MDUF
		if ( z_p == - L2( 1 ) ) break ;
#endif
		/*	adopt new answer top place from residue shift		*/
		y_p -= z_s ;					/* new ans top place */
	}							/* end of outer loop */
	/*	test 1 short, add 1, sub from residue	*/
	if							/* if dreg >= divisor */
		( ( y_p == bnlend ) && ( BNCPABS( w2  , z1 ) > - 1 ) )
		{ if ( ( y != NULL ) && ( bnadd_ul ( L2( 1 ) , y , bnend ) ) )     /* + 1 */
			{ FREE_BN_( zx ) ; FREE_BN_( w2 ) ; BNE1 ; } ;	/* return if o-f */
			bns_ub ( w2 , z1 , z1 ) ;				/* reduce dreg */
			-- y_p ; }
	/*	in case mod goes to source		*/
	BN_SIG( n = w [ S_PREC ] ^ x [ S_PREC ] ; )		/* hold div sign */
		/*	align residue for return		*/
		if ( z != NULL )					/* for mod */
		{ bns_down ( z_st , bnzx , bnzx , bnact + 1 ) ;
			bnclen ( z1 , z , bnact ) ;				/* out */
			BN_SIG( z [ S_PREC ] = x [ S_PREC ] ; ) }		/* mod sign */
#if B_ROUND
	/*	or round answer	*/
		else
		{ if ( ( y != NULL ) && ( iend < 0 ) )
			{ j = 0 ;
				if ( ! y_p )
				{ j = zx [ S_PREC ] & E_SZ( 0x8000 , 0x80000000 ) ; /* save top bit */
					bns_up ( L2( 1 ) , bnzx , bnzx , bnact + 1 ) ;
					-- y_p ; }
				if ( y_p == - L2( 1 ) )
				{
#if B_ROUND == 1
					if ( ( j || ( BNCPABS( w2 , z1 ) > - L2( 1 ) ) ) &&
							( bnadd_ul ( L2( 1 ) , y , 0 ) ) )
					{
#else
						if ( j || ( m = BNCPABS( w2 , z1 ) ) > - L2( 1 ) )
						{ if ( bnadd_ul ( ( blong )
									( ( ( y [ 0 ] ^ B_ROUND ) & 1 ) || ( m > 0 ) ) ,
									y , 0 ) )
#endif
						{ FREE_BN_( zx ) ; FREE_BN_( w2 ) ; BNE1 ; } } } } }	  /* o/f exit */
#endif
			BN_SIG( if ( y != NULL ) y [ S_PREC ] = n ; )		   /* div sign */
				FREE_BN_( zx ) ; FREE_BN_( w2 ) ;
			BNE0 ; }

#ifdef MDUF
#undef BNUF
#else
#undef bnact
#undef bnz
#undef bnufl
#endif

	/*	maths, int on big	*/

	/** div_bintoi and rawdivbintoi

	  div_bintoi handles error and trace
	  it calls rawdivbintoi
	  if error and trace code are not needed by definition,
	  the two functions merge and are defined to the same	*/

	R_D div_bintoi ( bele * x , int i , bele * y )
#if ( ERROR_MODE > 1 ) || ( defined BIG_TRACE )
	{ BN_TRAC( 'u' ) ;
		BN_ERR( if ) ( rawdivbintoi ( x , i , y ) )
			BNE_FUN( "overflow or divide by zero in div_bintoi" ) ;
		BN0 ; }

	BN_E rawdivbintoi ( bele * x , int i , bele * y )
#endif
	{ DEF_BN_( z ) ;
#ifdef USE_HEAP
		BN_ERR( int r ; ) BN_HEAP_( z ) ;
#endif
		bnflen ( z , S_LEN , ( bele ) 0 ) ;
		if ( i < 0 )
		{ z [ S_FRAC ] = ( bele ) - i ;
			BN_SIG( z [ S_PREC ] = 1 ; ) }				/* i becomes */
		else z [ S_FRAC ] = i ;					/* +/- z */
		BN_ERR( BN_NHEAP( return ) BN_HEAP( r = ) )
			( rawdiv_big_( x , z , y ) )
			BN_HEAP( ; free ( z ) ; BN_ERR( return ( r ) ) ) ; }

	/**	maths of int and blong on big	*/

	/**	the small add and subtract nest are here :

	  add_int   sub_int	add and sub ints
	  |     |
	  add_li   |     |		add from a blong
	  \  |     |
	  add_li_place	add from a blong to elements		
	  /    \
	  |	    |		choose on signs same or different
	  |	    |
	  bnadd_ul    bnsub_ul	unsigned add
	  or subtract with reversal if needed

	  bnadd_ul requires a ublong value to be any up to 0x F...F,0
	  bn_divide uses this to add an element length at any bit place
	  bnsub_ul requires a ublong value to be any up to 0x 1,0		*/

	R_D add_int ( int i , bele * x , bele * y )
	{ BN_TRAC( 'a' ) ;
		if ( x != y ) bnclen ( x , y , S_LEN ) ;
		BN_ERR( if ) ( add_li_place ( ( blong ) i , y , S_FRAC ) )
			BNE_FUN( "overflow in add_int" ) ;
		BN0 ; }

	R_D sub_int ( int i , bele * x , bele * y )
	{ BN_TRAC( 's' ) ;
		if ( x != y ) bnclen ( x , y , S_LEN ) ;
		BN_ERR( if ) ( add_li_place ( ( blong ) - i , y , S_FRAC ) )
			BNE_FUN( "overflow in sub_int" ) ;
		BN0 ; }

	BN_E add_li ( blong li , bele * x )
	{ BN_ERR( return ) ( add_li_place ( li , x , S_FRAC ) ) ; }

	BN_E add_li_place ( blong li , bele * x , int m )
	{ int j = 0 ;
#if SIGNED
		j = x [ S_PREC ] ^ ( li < 0 ) ;
#else
		j = ( li < 0 ) ;
#endif
		if ( li < 0 ) li = - li ;			/* treat negative li */
		if ( m >= ( S_PREC + j ) )
			return BN_ERR( ( ( int ) ( li != L2( 0 ) ) ) ) ;     /* too high to count */
		if ( ! j )
			/*	preprocess and call add		*/
		{ if ( m < - 1 ) return BN_ERR( ( 0 ) ) ;	/* u-f return */
			BN_ERR( return ) ( bnadd_ul ( ( ublong ) li , x , m ) ) ;
			BNDV ; }
		/*	preprocess and call sub		*/
		{ if ( li == ( E_SZ( 0x10000UL , L2( 0x100000000U ) ) ) )
			{ ++ m ; li = L2( 1 ) ; }
			if ( m < 0 ) BNE0 ;
			if ( ( m == S_PREC ) && ( bn_msx ( x , S_PREC ) == - 1 ) ) BNE0 ;
			bnsub_ul ( ( ublong ) li , x , m ) ; BNE0 ; } }

	/**	rawadd_ul adds ul to x at element place m
	  sign of x is ignored
	  blong must accomodate, not carry up, a carry from adding lower int
	  so max value in ul is [ 1...1 ] [ 0...0 ]
	  accepts m == -1 , it then ignores the u-f
	  returns 0 on success or u-f, 1 on o-f		*/

	int bnadd_ul ( ublong ul , bele * x , int m )
	{ if ( m >= S_PREC ) return ( 1 ) ;
		if ( m > - 1 )					/* low ul useable */
		{ ul += ( ublong ) x [ m ] ;
			x [ m ] = ( bele ) ul ; }
		ul >>= B_P_E ;
		if ( ++ m < S_PREC )
		{ ul += ( ublong ) x [ m ] ;
			x [ m ] = ( bele ) ul ; }
		else return ( ( int ) ( ( ublong ) ul != L2( 0 ) ) ) ;
		if ( ul > BNFUL )					/* if carry */
			while ( ( ++ m < S_PREC ) && ( ! ( ++ x [ m ] ) ) ) ;
		return ( m == S_PREC ) ; }

	/*	rawsub_ul subtracts ul from abs x at element place m
		ul max is [ 1 ] [ 0...0 ]
		sign of x changed by through zero	*/
	void bnsub_ul ( ublong ul , bele * x , int m )
	{ bele cy = 0 ;
		int n ;
		n = m ;
		if ( x [ m ] < ul )					/* if to carry */
			while ( ( ++ n < S_PREC ) && ! x [ n ] ) ;		/* test up */
		if ( n < S_PREC )					/* if number is there */
			bns_lipos ( ul , x , m ) ;				/* simple subtract */
		else							/* through zero */
		{ n = 0 ;
			while ( ! x [ n ] && ( n < m ) ) ++ n ;		/* pass lower zeroes */
			if ( n < m )
			{ x [ n ] = 0 - x [ n ] ; cy = E_SZ( 0xFFFF , 0xFFFFFFFF ) ;
				while ( ++ n < m )				/* complement ints */
					x [ n ] = ~ x [ n ] ; }				/* through active */
			x [ m ] = ( bele )
				( ( ublong ) ul - x [ m ] + cy ) ;		/* subtract with cy */
			BN_SIG( x [ S_PREC ] = ! x [ S_PREC ] ; ) } }	/* through zero */

	/*	bns_lipos subtracts ul at m when result will not pass through zero
		used by bnsub_ul and bn_readyx		*/
	void bns_lipos ( ublong ul , bele * x , int m )
	{ x [ m ] = ( bele ) ( ul = ( ublong ) x [ m ] - ul ) ;
		if ( ul > E_SZ( 0xFFFFUL , L2( 0xFFFFFFFFU ) ) )
			while ( ( x [ ++ m ] -- == ( bele ) 0 ) &&
					( m < ( S_PREC - 1 ) ) ) ; }			/* cy */

		/**	mult_int
		  |
		  mult_li
		  |
		  bnmulul		*/

		R_D mult_int ( int i , bele * x , bele * y )		/* x = x * n */
		{ BN_TRAC( 'm' ) ;
			BN_ERR( if )
				( mult_li ( ( blong ) i , x , y ) )
				BNE_FUN( "overflow in mult_int" ) ;
			BN0 ; }

		BN_E mult_li ( blong li , bele * x , bele * y )
		{ BN_ERR( if  )
			( bnmulul ( ( ublong ) ( li < L2( 0 ) ? - li : li ) ,
						x , y , S_PREC ) )
				BN_ERR( return ( 1 ) ) ;
			BN_SIG( y [ S_PREC ] = x [ S_PREC ] ^ ( li < L2( 0 ) ) ; )
				BNE0 ; }

		unsigned bnmulul ( ublong ul , bele * x , bele * y , int sz )
		{ ublong cy = L2( 0 ) ;
			while ( sz -- )
				* ( y ++ ) = ( bele )
					( ( ublong ) ( cy = ( cy >> B_P_E ) + * ( x ++ ) * ul ) ) ;
			return ( ( unsigned int )
					( ( ublong ) cy >> B_P_E ) ) ; }    /* return the overflow */

			/** int and blong divides

			  div_int		div_i_4
			  |		   |
			  div_li		div_li_4
			  \        /
			  bndivul			*/

			R_D div_int ( int i , bele * x , bele * y )
			{ BN_TRAC( 'd' ) ;
				BN_ERR( if ) ( div_li ( ( blong ) i , x , y ) )
					BNE_FUN( "divide by zero in div_int" ) ;
				BN0 ; }

#if ! ( FRAC )
			R_D div_i_4 ( int i , bele * x , bele * y , int * rem )
			{ blong lr ;
				BN_TRAC( 'j' ) ;
				BN_ERR( if ) ( div_li_4 ( ( blong ) i , x , y , & lr ) )
					BNE_FUN( "divide by zero in div_i_4" ) ;
				* rem = ( int ) lr ;
				BN0 ; }
#endif

			BN_E div_li ( blong li , bele * x , bele * y )
			{
#if ! ( FRAC )
				blong dummy ;
				BN_ERR( return ) ( div_li_4 ( li , x , y , & dummy ) ) ; }

			BN_E div_li_4 ( blong li , bele * x , bele * y , blong * li_rem )
			{
#endif
				ublong u ;
#if ! ( FRAC ) || B_ROUND
				ublong r ;
#endif
				if ( ! li ) { BN_NFRAC( * li_rem = L2( 0 ) ; ) BNE1 ; }
				u = li < L2( 0 ) ? - li : li ;
#if ! ( FRAC ) || B_ROUND
				r =
#endif
					bndivul ( L2( 0 ) , u , x , y , S_PREC ) ;
#if B_ROUND
				bnadd_ul (
#if B_ROUND == 1
						( r << L2( 1 ) ) >= ( blong ) u
#elif B_ROUND > 1
						( ( r = r << L2( 1 ) ) > u ) ||
						( ( r == u ) && ( y [ 0 ] ^ B_ROUND ) & 1 )
#endif
						, y , 0 ) ;
#endif
				BN_NFRAC( * li_rem = ( int ) ( BN_SIG( x [ S_PREC ] ? - r : ) r ) ) ;
				BN_SIG( y [ S_PREC ] = x [ S_PREC ] ^ ( li < L2( 0 ) ) ; )
					BNE0 ; }

			/**	bndivul does all datum divides and is used for decimal converions
			  ul is the divisor
			  cy is the carry in, this is divided into the number by ul
			  the function returns the final residue		*/

			unsigned int bndivul
				( ublong cy, ublong ul, bele * x, bele * y, int sz)
				{ while ( sz -- )
					{ y [ sz ] = ( bele )
						( ( ublong ) ( cy = ( cy << B_P_E ) + x [ sz ] ) / ul ) ;
						cy = cy % ul ; }
					return ( ( unsigned int ) cy ) ; }			/* return residue */

			/**	shifts

			  big_shift
			  /     \
			  bns_up	bns_down

			  bns_up		is called by 	bndivul, bn_divide ( 3 calls )
			  bns_down			bndivul, bn_divide & bnin2	*/

			R_D big_shift ( blong shift , bele * x , bele * y )
			{ blong li ;
#if B_ROUND
				ublong lh ;
#if B_ROUND > 1
				ublong lj ; bele mask = ( unsigned ) - 1 ; int m ;
#endif
#endif
				BN_TRAC( 'z' ) ;
				li = bn_msx ( x , S_PREC ) + L2( 1 ) ;
				if ( shift < L2( 0 ) )
				{		/*	shift down		*/
					if ( ( shift = - shift ) <= li )		/* shift to 0 then round */
					{
#if B_ROUND
						lh = shift - L2( 1 ) ;				/* to first u-f */
						lh = ( blong ) ( ( bele )
								x [ ( int ) ( ( blong ) lh >> BNSHIFT ) ] >>
								( ( bele ) lh & BNMASK ) ) & L2( 1 ) ;
#if B_ROUND > 1
						/* set lj = 1 if rounding is certain		*/
						if ( ( shift > L2( 1 ) ) && lh )		/* if rounding is possible */
						{ lj = shift - L2( 1 ) ;
							mask >>= ( B_P_E - ( lj & BNMASK ) ) ;
							lj = ( blong ) ( ( bele )
									x [ m = ( unsigned int ) ( ( blong ) lj >> BNSHIFT ) ]
									& mask ) ;
							if ( ! lj )
								while ( m -- && ! x [ m ] ) ; lj = ( m > - 1 ) ; }	/* l to r */
						else lj = L2( 0 ) ;
#endif
#endif
						bns_down ( shift , x , y , S_PREC ) ;
#if B_ROUND
						bnadd_ul (
#if B_ROUND > 1
								( ( ( y [ 0 ] ^ B_ROUND ) & 1 ) || lj ) &&
#endif
								lh , y , 0 ) ;				/* cannot o-f */
#endif
					}
					else bnflen ( y , S_LEN , ( bele ) 0 ) ; }	/* end of shift down */
				else
				{		/*		shift up		*/
					if ( shift <= ( ( blong ) S_PREC * B_P_E ) - li )
						bns_up ( shift , x , y , S_PREC ) ;
					else
					{ if ( ! li )  bnflen ( y , S_LEN , ( bele ) 0 ) ;
						BN_ERR( else ) BNE_FUN( "overflow in big_shift" ) ; } }
				BN_SIG( y [ S_PREC ] = x [ S_PREC ] ; )
					BN0 ; }

			/**	bns_up and bns_down
			  will crash the application if shift or m exceed S_PREC * B_P_E
			  shift ,   source ,   dest   , size		*/

			void bns_up ( ublong shift , bele * x , bele * y , int m )
			{ unsigned u , d ; int n , p ; bele * z ;
				d = B_P_E - ( u = ( unsigned int ) shift & BNMASK ) ;	/* bits : up down */
				n = ( int ) ( ( ublong ) shift >> BNSHIFT ) ;		/* n ints */
				z = y + n ;						/* dest offset */
				p = m - n ;						/* active shift */
				if ( u )						/* if bit shift */
				{ while ( -- p )
					z [ p ] = ( x [ p ] << u ) + ( x [ p - 1 ] >> d ) ;
					* z = * x << u ; }
				else							/* element shift */
				{ while ( p -- ) z [ p ] = x [ p ] ; }
				while ( n -- ) y [ n ] = 0 ; }			/* fill low */

			void bns_down ( ublong shift , bele * x , bele * y , int m )
			{ int p = - 1 , n , d , q , u ; bele * z ;
				u = B_P_E - ( d = ( unsigned int ) shift & BNMASK ) ;	/* bits : down up */
				n = ( int ) ( ( ublong ) shift >> BNSHIFT ) ;		/* n ints */
				q = m - ( int ) ( ( ublong ) shift >> BNSHIFT ) - 1 ;
				z = x + n ;						/* source offset */
				if ( d )
				{ while ( ++ p < q )
					y [ p ] = ( z [ p ] >> d ) + ( z [ p + 1 ] << u ) ;
					y [ p ] = z [ p ] >> d ; }
				else
				{ ++ q ; while ( ++ p < q ) y [ p ] = z [ p ] ; -- p ; }
				while ( ++ p  < m ) y [ p ] = 0 ; }

			/**	maths auxiliaries	*/

			/**		bit size
			  primary		tech
			  mssb	  _
			  overbit	  _\_  bn_msx	if ! ( FRAC ) ntop is defined as mssb
			  ntop	  _/

			  bit comparisons
			  _ bn_ubsum
			  obitdif	   _/
			  \_ bn_ubdif
			  a_obitdif  _/				*/

			blong overbit ( bele * x )
#if SIGNED
			{ blong li ;
				BN_NSIG( return ) ( li = bn_msx ( x , S_PREC ) + 1 ) ;
				BN_SIG( return ( x [ S_PREC ] ? - li : li ) ) ; }

			blong bnbn_uobit ( bele * x )
#endif
			{ return ( bn_msx ( x , S_PREC ) + 1 ) ; }

			/* a_obitdiff ( x , y ) returns the absolute value of the unsigned
			   over bit difference of x and y			*/
			blong a_obitdif ( bele * x , bele * y )
			{ blong li ;
				li = bn_ubdif ( x , y ) ;
				return ( li < 0 ? - li : li ) ; }

			blong obitdif ( bele * x , bele * y )
#if SIGNED
			{ blong li ;
				li = ( x [ S_PREC ] == y [ S_PREC ] ) ?
					bn_ubdif ( x , y ) : bn_ubsum ( x , y ) ;
				return ( y [ S_PREC ] ? - li : li ) ; }

			/**	finds overbit of ( x - y ) when these have different signs */

			blong bn_ubdif ( bele * x , bele * y )
#endif
			{ blong k , li , mask ; int m = S_PREC - 1 ;
				unsigned sig ; bele * gt , * lt ;
				while ( ( x [ m ] == y [ m ] ) && ( m > - 1 ) ) -- m ; /* while ints == */
				if ( m == - 1 ) return ( L2( 0 ) ) ;			/* return if equal */
				if ( x [ m ] > y [ m ] ) { gt = x ; lt = y ; sig = 1 ; } /* set pointers */
				else { gt = y ; lt = x ; sig = 0 ; }
				/*	find end of carry down		*/
				if ( ( li = gt [ m ] - lt [ m ] ) == 1 )		/* test & set li */
				{ -- m ;
					while ( ( gt [ m ] == 0 ) &&
							( lt [ m ] == E_SZ( 0xFFFF , 0xFFFFFFFF ) ) && ( m > - 1 ) ) -- m ;
					if ( m == - 1 ) return ( sig ? - L2( 1 ) : L2( 1 ) ) ;
					li = E_SZ( 0x10000UL , L2( 0x100000000U ) ) +		/* set li */
						gt [ m ] - lt [ m ] ; }
				k = ( blong ) B_P_E * ( m + 1 ) ;
				mask = E_SZ( 0xFFFF8000UL , L2( 0xFFFFFFFF80000000U ) ) ;
				while ( ! ( li & mask ) ) { mask = mask >> 1 ; -- k ; }
				/*	under the carry, is lt > gt	*/
				if ( ( mask | ( blong ) gt [ m ] ) == ( mask | ( blong ) lt [ m ] ) )
				{ -- m ;
					while ( ( gt [ m ] == lt [ m ] ) && ( m > - 1 ) ) -- m ; }
				k = k - ( ( lt [ m ] > gt [ m ] ) && ( m > - 1 ) ) ;	/* adjust */
				return ( sig ? - k : k ) ; }

#if SIGNED

			/** a_obitsum enables obitdiff to return the overbit of a difference when
			  the argument values are of different signs
			  it measures the overbit of the sum
			  as it is called only by obitdiff it should be within there
			  but I am not going to fix that			*/

			blong bn_ubsum ( bele * x , bele * y )
			{ blong li ; ublong k ; int j , m ;
				m = S_PREC - 1 ;
				while ( ! ( x [ m ] || y [ m ] ) && ( m > - 1 ) ) -- m ; /* find topmost */
				k = ( blong ) x [ m ] + y [ m ] ;
				li = ( blong ) m << E_SZ( 4 , 5 ) ; j = 0 ;		/* save int place */
				while ( k && ( k & 1 ) ) { k = k >> 1 ; ++ j ; }
				if ( k )						/* cannot carry */
				{ while ( k ) { k = k >> 1 ; ++ j ; } }
				else
				{ -- m ;
					while ( ( ( k = ( x [ m ] + y [ m ] ) ) ==
								E_SZ( 0xFFFF , ( int ) 0xFFFFFFFF ) ) && ( m > - 1 ) )
						-- m ;						/* while may carry */
					if ( k > E_SZ( 0xFFFF , ( int ) 0xFFFFFFFF ) ) k = 1 ; else k = 0 ; }
				return ( li + j + k ) ; }
#endif

			/** mssb ( x ) returns the bit place of the most significant set bit
			  of the absolute value of x
			  and
			  if the compilation is signed, if x == 0 the sign is set positive

			  mssb
			  |
			  bn_msx			*/

			/*  mssb is called by	rawmult_big (2), kb_2_big_2, bnnum_here & bn_s_out  */
			blong mssb ( bele * x )
			{
#if SIGNED
				blong lh ;
				if ( ( lh = bn_msx ( x , S_PREC ) ) == - L2( 1 ) )
					x [ S_PREC ] = 0 ; return ( lh ) ; }			/* correct - 0 , ret */
#else
			return ( bn_msx ( x , S_PREC ) ) ; }
#endif

			/*  bn_msx is called by 	bn_divide (3), add_li+place, big_shift, overbit,
				bnbn_uobit, mssx, ntop & bn_m_00	*/
blong bn_msx ( bele * x , int sz )
{ bele be ; blong ul = - L2( 1 ) ;
	-- sz ;
	while ( ( sz > - 1 ) && ( ! x [ sz ] ) ) -- sz ;
	if ( sz == - 1 ) return ( - L2( 1 ) ) ;
	be = x [ sz ] ;
	while ( be ) { be = be >> 1 ; ++ ul ; }		/* find top bit */
	return ( ( blong ) ( ( ublong ) sz << BNSHIFT ) + ul ) ; }

#if FRAC
blong ntop ( bele * x )
{ return ( bn_msx ( x , S_PREC ) - ( blong ) S_FRAC * B_P_E ) ; }
#endif

int cp_abs ( bele * w , bele * x )
{			/* returns ( abs ( x ) - abs ( w ) ) as - 1 , 0 , 1  */
#ifndef MDUF
	int m = S_PREC ;
#else
	return ( bncp_abs ( w , x , S_PREC ) ) ; }

int bncp_abs ( bele * w , bele * x , int m )
{
#endif
	while ( ( -- m > - 1 ) && ( w [ m ] == x [ m ] ) ) ;
	return ( ( int ) m == - 1 ? 0 : w [ m ] > x [ m ] ? - 1 : 1 ) ; }

	/**	input and output	*/

	/**	bnin1 permits loading to a length of S_PREC + 1 ,
	  needed for pix.txt 

	  bnin1 finds the mark and sign then finds if decimal or hex
	  provides error response
	  copies the arguments into a structure
	  | \
	  |  for decimal to bn_dec_in which makes a buffer for the f-p
	  |  /
	  bnin2, which processes its way through the input stream
	  collecting each next character from bn2i
	  hex input is planted into the top of the destination
	  until o-f or end of i-p
	  then shifted down to place
	  and the f-p is collected and planted
	  see comments for i-p and f-p of decimal input below
	  inserting into i-p and f-p uses max power of 10 within an element

	  bn_dec_in_ip and bn_dec_in_fp serve big_in and bnnum_here    */


R_D big_in ( FILE * sm , int mark , bele * x , int base , int b_end )
#if ( ERROR_MODE > 1 ) || ( FRAC && SIGNED )
{ BN_ERR( if ) ( bnin1 ( sm , mark , x , base , b_end
#if ( FRAC && SIGNED )
			, 0
#endif
			) )
	BNE_FUN( "fault in file input to big_in" ) ;
BN0 ; }

BN_E bnin1 ( FILE * sm , int mark , bele * x , int base , int b_end
#if ( FRAC && SIGNED )
		, int xtra		/* xtra adds to the length of the target */
#endif
		)
#endif
{ int ich = 256 , bnerr ;
	BN_SIG( int sig = 0 ; )
		BN_DEC( struct bn_in sc ; sc.ulbuff = L2( 0 ) ; )
		if ( sm == NULL ) BNE1 ;
	/*	find mark	*/
	if ( mark )
		while ( ( ( ich = fgetc ( sm ) ) != mark ) && ( ich != EOF ) )
#ifdef SHO_LEAD_IN
			if ( ich > 31 ) putc( ich , FILE_SM )
#endif
				;
	if ( ich == EOF ) BNE1 ;
	/*	read start of number	*/
	ich = bn2i ( sm , b_end ) ;
	/*	take sign		*/
	if ( ( ich == '+' ) || ( ich == '-' ) )
	{ BN_SIG( if ( ich == '-' ) sig = 1 ; )		/* nor end yet */
		ich = bn2i ( sm , b_end ) ; }
	/*	try to find 0x	*/
	bnflen ( x , S_LEN , ( bele ) 0 ) ;
	if ( ich == '0' )
	{ if ( ( ich = bn2i ( sm , b_end ) ) == 'x' )
		{ base = 16 ; ich = '0' ; }
		else
			if ( ich < 0 ) return BN_ERR( ( ich != - 2 ) ) ; }	/* EOF or terminator */
	bnerr = ( ! base ) ;					/* not lost here */

	/*	call bn_dec_in or bnin & error response		*/
	if ( (
#ifndef NO_DEC
				base == 10 ?

				bn_dec_in ( sm , x , ich , b_end ) :
#endif
				bnin2 ( sm , x , 16 , ich , b_end
#if ( FRAC && SIGNED )
					, xtra
#endif
#ifndef NO_DEC
					, & sc
#endif
					)
		 ) | bnerr ) BNE1 ;
	BN_SIG( x [ S_PREC BN_FRAC( + xtra ) ] = sig ; ) BNE0 ; }

#ifndef NO_DEC

	/**	make a decimal buffer to collect extra length of f-p,
	  this is 3 / 16 length of f-p + 2
	  call bnin2, convert from buffer			*/

int bn_dec_in ( FILE * sm , bele * x , int ich , int b_end )
{ struct bn_in sc ;
#ifdef USE_HEAP
	int bnerr = 0 ;
#if FRAC
	bele * d_hold ;
	if ( ( d_hold = ( bele * ) bn_h ( ( int ) ( ( blong )
						( L2( 3 ) * FRAC_CHAR ) / L2( 16 ) ) + L2( 2 ) , BY_P_E ) )
			== NULL ) return ( 1 ) ;
#endif
#else
#if FRAC
	bele d_hold [ ( int ) ( ( blong ) ( L2( 3 ) * BNASZ( FRAC ) ) /
			L2( 16 ) + L2( 2 ) ) ] ;
#endif
#endif
	sc.x = x ; BN_FRAC( sc.h = d_hold ; ) sc.ulbuff = L2( 0 ) ; sc.n = L2( 0 ) ;
	sc.m = 1 ;
	BN_HEAP( bnerr = ) BN_NHEAP( return )
		( bnin2 ( sm , x , 10 , ich , b_end ,
#if ( FRAC && SIGNED )
				  0 ,
#endif
				  & sc ) ) ;
	BN_FRAC( FREE_BN_( d_hold ) ; )
		BN_HEAP( return ( bnerr ) ) ; }
		/* vv endif # if NO_DEC */
#endif

		/*	ich contains first character taken, or point	*/
int bnin2 ( FILE * sm , bele * x , int base , int ich , int b_end
#if ( FRAC && SIGNED )
		, int xtra
#endif
#ifndef NO_DEC
		, struct bn_in * sc
#endif
		)
{ blong li ;
	int bnerr = 0 ; int got_n = 0 ;			/* bnerr is used */
	BN_DEC( int base_b ; )
		/*	pass leading zeros	*/
		while ( ich == '0' )
		{ got_n = 1 ; ich = bn2i ( sm , b_end ) ; }
	/*	initialise	*/
#ifndef NO_DEC
	base_b = ( base == 16 ) ;				/* 1 for hex */
	if ( base == 10 ) li = INT_CHAR ;
	else
#endif
		li = BNXICH ;						/* integer digits */
	/*	take values of integer part	*/
	while ( ( bn_n_( ich ) < base ) && li -- )
	{ got_n = 1 ;
		BN_DEC( if ( base_b ) )
			bnplant ( ( unsigned ) ich ,
					& x [ S_FRAC BN_FRAC( BN_SIG( + xtra ) ) ] , li ) ;
#ifndef NO_DEC
		else
			bnerr |= ( bn_dec_in_ip ( ich , sc ) ) ;
#endif
		ich = bn2i ( sm , b_end ) ; }	/* li is -1 at full end or too blong */
#ifndef NO_DEC
	if ( ! base_b )
	{ if ( bn_decip_end ( sc ) ) return ( 1 ) ; }		/* return on i-p o-f */
	else
#endif
		if ( li != BNXICH )
			bns_down ( li << L2( 2 ) , x + S_FRAC BN_SIG( BN_FRAC( + xtra ) ) ,
					x + S_FRAC BN_SIG( BN_FRAC( + xtra ) ) , S_PREC BN_FRAC( - S_FRAC ) ) ;
	if ( ich == '.' )
	{ ich = bn2i ( sm , b_end ) ;
		/*	take fraction part	*/
#if FRAC
#ifndef NO_DEC
		if ( ! base_b )
		{ sc->ulbuff = sc->n = L2( 0 ) ; sc->m = S_FRAC ;
			li = FRAC_CHAR ; }
		else
#endif
			li = ( blong ) ( S_FRAC BN_SIG( BN_FRAC( + xtra ) ) ) * BNXPE ;
		/* fraction digits */
		while ( ( bn_n_( ich ) < base ) && li -- )		/* number and wanted */
		{ BN_ERR( got_n = 1 ; )
			BN_DEC( if ( base_b ) ) bnplant ( ( unsigned ) ich , x , li ) ;
			BN_DEC( else bn_dec_in_fp ( ich , sc ) ; )
				ich = bn2i ( sm , b_end ) ; }
		BN_DEC( if ( ! base_b ) bn_decfp_end ( sc ) ; )
#endif
	}		/*	 o-f or fraction u-f  	*/
	while ( bn_n_( ich ) < base  ) ich = bn2i ( sm , b_end ) ;
	BN_ERR( if ( ich != -2 ) bnerr = 1 ; )
		/*	finish		*/
		return ( ( bnerr && b_end ) | -- got_n ) ; }		/* DO I NEED bnerr ? */

		/** i_end is to equal one of :	ascii terminator
		  EOF terminator
		  0 , which requires 10 10
		  10 , which requires 10 10
		  in the same block of white space
		  13 one 10 as terminator
		  256 as impossible value to prevent termination

		  terminator values :

		  0	non parsing terminates
		  10	10 ( white space ignored ) 10
		  13	10
		  etc	accurately			*/

int bn2i ( FILE * sm , int i_end )     	/* function for big_in */
{ int ich , rct ;
	if ( ! i_end ) i_end = 10 ;
	rct = ( i_end != 10 ) ;
	if ( i_end == 13 ) i_end = 10 ;
	do
	{ ich = fgetc ( sm ) ;
		if ( ( ich == i_end ) && rct ++ ) return ( - 2 ) ; }
	while ( ( ich == ' ' ) || ( ( unsigned ) ( ich - 9 ) < 5 ) ) ;
	return ( ich ) ; }

void bnplant ( unsigned ich , bele * x , blong li )
{ ich = bn_n_( ich ) ;
	x [ ( int ) ( ( blong ) li / BNXPE ) ] |=
		( bele ) ( ich <<
				( ( ( unsigned int ) li & E_SZ( 3L , L2( 7 ) ) ) * L2( 4 ) ) ) ; }

#ifndef NO_DEC
		/**	conversion is based upon the struct bn_in
		  bn_dec_in_ip ( ich , & sc ) accumulates inserts digital values
		  bn_decip_end ( & sc ) inserts any residue

		  this and bn_dec_in_fp are for use by big_in , kb_2_big , num_here    */

int bn_dec_in_ip ( int ich , struct bn_in * sc )	/*	OK	*/
{ sc->ulbuff = sc->ulbuff * L2( 10 ) + ich - L2( 48 ) ;
	if ( ( ++ sc->n ) % E_SZ( 4 , 9 ) ) return ( 0 ) ;
	if ( ( bnmulul ( E_SZ( 10000UL , L2( 1000000000U ) ) ,
					sc->x + S_FRAC , sc->x + S_FRAC , sc->m ) ? 1 :		/* I know */
				bnadd_ul ( sc->ulbuff , sc->x , S_FRAC ) ) ) return ( 1 ) ;
	if ( sc->x [ S_FRAC + sc->m - 1 ] &&
			( sc->m < ( S_PREC BN_FRAC( - S_FRAC ) ) ) )
		++ sc->m ;
	sc->ulbuff = L2( 0 ) ; return ( 0 ) ; }

int bn_decip_end ( struct bn_in * sc )	/*	OK	*/
{ ublong li = L2( 1 ) ;
	if ( ( sc->n % E_SZ( 4 , 9 ) ) )
	{ while ( ( sc->n -- ) % E_SZ( 4 , 9 ) ) li *= L2( 10 ) ;
		if ( ( bnmulul ( li , sc->x + S_FRAC ,
						sc->x + S_FRAC , sc->m ) ? 1 :

					bnadd_ul ( sc->ulbuff , sc->x , S_FRAC ) ) ) return ( 1 ) ; }
		return ( 0 ) ; }

		/**	fraction collection from an incoming stream of decimal digits
		  bn_dec_in_fp ( ich , sc ) saves digital values into an int buffer
		  E_SZ ( four , nine ) digits are saved
		  one int is saved in tail of x for every three saved into sc->h
		  bn_decfp_end ( sc ) saves any residue then inserts all		*/
#if FRAC
void bn_dec_in_fp ( int ich , struct bn_in * sc )
{ sc->ulbuff = sc->ulbuff * L2( 10 ) + ich - L2( 48 ) ;
	if ( ++ sc->n % E_SZ( 4 , 9 ) ) return ;
	if ( sc->n % E_SZ( 16 , 36 ) )
		* sc->h ++ = ( bele ) sc->ulbuff ;		 /* store 3 in hold */
	else
	{ * sc->x ++ = ( bele ) sc->ulbuff ; -- sc->m ; } /* store 1 in x */
	sc->ulbuff = L2( 0 ) ; }

void bn_decfp_end ( struct bn_in * sc )
{ ublong ul ;
#if BIG_DAJ == 1
	unsigned int rem = 0 ;
#endif
	while ( sc->n % E_SZ( 4L , L2( 9 ) ) ) bn_dec_in_fp ( '0' , sc ) ;
	/* stuff until int is full */
	sc->n /= E_SZ( 4L , L2( 9 ) ) ;
	while ( sc->n )
	{ if ( sc->n -- % L2( 4 ) ) { -- sc->h ; ul = * sc->h ; }
		else
		{ -- sc->x ; ++ sc->m ; ul = ( blong ) * sc->x ; * sc->x = 0 ; }
#if BIG_DAJ == 1
		rem |=
#endif
			bndivul ( ul , E_SZ( 10000UL , L2( 1000000000U ) ) ,
					sc->x , sc->x , sc->m ) ; }
#if BIG_DAJ == 1
		bnadd_ul ( ( blong ) ( rem != 0 ) , sc->x , 0 ) ;
#endif
}
/* v end # if FRAC */
#endif
/* v end # ifndef NO_DEC */
#endif

/*	kb_2_big_2
	this accepts kb input
	if a console interface has been included
	non parsing keys are refused
	on i-p count it calls bnnum_here to test for overflow
	if no console interface
	text is collected in a buffer
	then converted by num_here			*/

int kb_2_big ( bele * dest )
{ return ( kb_2_big_2 ( dest , 10 ) ) ; }

int kb_2_big_2 ( bele * dest , int base )
#if ( defined GET_C_POS ) && ( defined GET_CHE )
{ int ich , k = 0 , ok_char = 0 , sigot = 0 , gotx = 0   , base2 , ypos ;
	/*	    count ,   valid = 1 , got sign  , got 0x = 2 , base   pos in y */
	/* longs for comparisons */
	blong digs = L2( 0 ) ;			/* digits */
	BN_FRAC( blong p_got = 0 ; )			/* got point flag */
		DEF_BN_( hold ) ;				/* permits [ Esc ] */
	char * q ;
#ifdef USE_HEAP
	char * asc_in ;
	BN_HEAP_( hold ) ;
	asc_in = ( char * ) bn_h ( ( int ) BNKB_CH + 1 , 1 ) ;
#else
	char asc_in [ BNKB_CH + 1 ] ;			/* assemble ascii */
#endif
	q = asc_in ; base2 = ! base ? 16 : base ;
	ypos = READ_LINE( GET_C_POS ) ;
	* q = 0 ;
	while ( 1 )					/* keystrokes */
	{ q [ 1 ] = 0 ;
		* q = ( ich = GET_CH ) ;			/* get and plant */
		if ( ( ich > 31 ) && ( ich < 127 ) ) putchar( ich ) ;
		ok_char = ( k < BNKB_CH ) ;			/* prevent buffer o-f */
		if ( ok_char &&
				! ( base || gotx ) )			/* test if 0x forced by call */
		{ switch ( ich )
			{ BN_FRAC( case '-' : )
				case '+' : break ;
				case '0' : if ( k != sigot ) ok_char = 0 ; break ;
				case 'X' :
				case 'x' : if ( k != ( sigot + 1 ) ) ok_char = 0 ; break ;
				default  : ok_char = 0 ; } }
						   if ( ok_char || ( ich < 32 ) )
						   { if ( bn_n_( ich ) < base2 )			/* if a digit */
							   ok_char =						/* o-f or u-f */
#if FRAC
								   p_got ? p_got < ( (			/* f-p underflow */
#ifndef NO_DEC
											   base2 == 10 ? FRAC_CHAR :
#endif
											   S_FRAC * E_SZ( 4 , 8 ) ) + 2 ) :
#endif
									   (					/* i-p overflow */
#ifndef NO_DEC
															base2 == 10 ?
															( digs < ( INT_CHAR - 1 ) ) ||
															! bnnum_here ( asc_in , base2 , hold BNA_1 ) :
#endif
															digs < ( ( S_PREC - S_FRAC ) * E_SZ( 4 , 8 ) ) ) ;
							   else						/* not a digit */
							   { switch ( ich )
								   { case 'X' :
									   case 'x' : if ( ( k == 1 + sigot ) &&
														  ( asc_in [ sigot ] == '0' ) )
												  { base2 = 16 ; gotx = 2 ; digs = - 1 ; } /* !!!! */
												  else ok_char = 0 ;  break ;
												  /*	base2  offset into string	*/
												  BN_SIG( case '-' : )
									   case '+' : if ( ! k ) sigot = 1 ; else ok_char = 0 ; break ;
													  BN_FRAC(
															  case '.' : if ( ! p_got ) p_got = 1 ; else ok_char = 0 ; break ; )
															  case ENTER_K :
																			 if ( k > ( sigot BN_FRAC( + ( p_got > 0 ) ) + gotx ) )
																			 { bnnum_here			/* convert & issue */
																				 ( asc_in + sigot + gotx , base2 , hold BNA_1 ) ;
																				 bnclen ( hold , dest , S_LEN ) ;
																				 BN_SIG( dest [ S_PREC ] = ( * asc_in == '-' ) ; )
																					 BN_SIG( mssb ( dest ) ; )
																					 FREE_BN_( hold ) ; FREE_BN_( asc_in ) ;
																				 putchar( '\n' ) ; return ( 0 ) ; }
																			 break ;
															  case 27 : FREE_BN_( hold ) ; FREE_BN_( asc_in ) ; return ( - 1 ) ;
															  case BACK_SP  : if ( k )
																			  { -- k ; -- q ;
																				  switch ( * q )

																				  { case 'X' :
																					  case 'x' : digs = 1 ; gotx = 0 ; base2 = base ;
																								 break ;
																								 BN_FRAC( case '.' : p_got = 0 ; break ; )
																									 BN_SIG( case '-' : )
																					  case '+' : sigot = 0 ; break ;
																					  default  : BN_FRAC( if ( p_got ) -- p_got ; else )
																								 if ( digs ) -- digs ;	/* digit */
																				  } }			/* end switch * q , if ( k ) */
																			  ok_char = - 1 ; break ;	/* - 1 to supress bell */
															  default  : ok_char = 0 ; } } }	/* else , switch , if */
																		 /*	actions		*/
																		 if ( ok_char < 1 )
																		 {
#ifndef NO_BELL
																			 if ( ! ok_char ) putchar( '\a' ) ;
#endif
																			 * q = 0 ;
																			 SET_C_POS_( ypos , 1 ) ; printf( "%s " , asc_in ) ;
																			 SET_C_POS_( ypos , 1 ) ; printf( "%s" , asc_in ) ; }
																		 else
																		 { BN_FRAC( if ( p_got ) ++ p_got ; else ) ++ digs ;
																			 ++ q ; ++ k ; }
	} }					/* end of while ( 1 ) , fn */
#else
{ char asc_in [ 81 ] ;					/* alternative fn */
	printf ( "\t80 characters maximum\n" ) ;
	scanf( "%80s" , asc_in ) ;
	bnnum_here ( asc_in , base , dest BNA_1 ) ;
	return ( 1 ) ; }
#endif

	/** num_here converts a decimal text string to a big number,
	  terminates ok on any character that does not parse for the conversion
	  num_here responds to overflow or no number as an error
	  num_here calls bnnum_here setting base 10		*/

	R_D num_here ( char * sg , bele * x )		/* put a value into x */
{ BN_ERR( if )
	( bnnum_here ( sg ,
				   ( ( sg [ 1 ] | 32 ) == 'x' ) || ( ( sg [ 2 ] | 32 ) == 'x' ) ?
				   12 : 10 , x BNA_1 ) )
		BNE_FUN( "num_here   " ) ;
	BN0 ; }

	/** to provide a routine that tests and converts a string number to binary
	  it returns 0 if it finds a number or 1 if overflow or no number
	  bnnum_here advances a pointer through :
	  leading spaces		- ignored
	  if SIGNED , a sign	- adopted
	  spaces			- ignored
	  optional 0x		- set base to 16
	  spaces			- ignored
	  leading zeros		- noted as a number, otherwise ignored
	  if base is 16 a direct hex conversion to base 16 is used is made
	  the integer of the number is measured to find point or low end
	  the hex digits are converted into their places
	  below the point the hex digits are converted to their places
	  if base is 10
	  if FRAC it :
	  tests for a decimal point, if yes
	  saves & x { S_FRAC ]
	  runs to the end of the ascii
	  steps back along ascii ch by ch repeatedly until decimal point
	  puts value from ch in x [ S_FRAC ]
	  divides the unit.fraction by base
	  restores x [ S_FRAC ]
	  tests if wanted and appropriate and increments number		*/

int bnnum_here ( char * sg , int base , bele * x
#if ( BIG_DAJ == 1 ) && ( ! defined NO_DEC )
		, int asc_inc
#endif
		)
{ int pos = 0 ; char * s2 ; bele * x2 ;
	BN_FRAC( BN_DEC( blong j = 0 ; ) unsigned int n ; )
		/* characters ; digit value */
		BN_SIG( int sig = 0 ; )			/* sign */
#if ( BIG_DAJ == 1 ) && ( ! defined NO_DEC )
		int rem = 0 ;
#endif
#ifndef NO_DEC
	struct bn_in sc ;
	sc.x = x ; BN_FRAC( sc.h = NULL ; ) sc.ulbuff = L2( 0 ) ;
	sc.n = L2( 0 ) ; sc.m = 1 ;
#endif
	bnflen ( x , S_LEN , ( bele ) 0 ) ; x2 = x ;
	while ( * sg == ' ' ) ++ sg ;			/* tolerate leading spaces */
	BN_SIG( if ( * sg == '-' ) { ++ sg ; sig = 1 ; } else )
		if ( * sg == '+' ) ++ sg ;
	while ( * sg == ' ' ) ++ sg ;			/* tolerate leading spaces */
	if ( ( * sg == '0' ) && ( ( sg [ 1 ] | 32 ) == 'x' ) )
	{ base = 16 ; sg = sg + 2 ; while ( * sg == ' ' ) ++ sg ; }
	if ( ! base ) return ( 1 ) ;
	while ( * sg == '0' ) ++ sg ;			/* leading zeroes */
#ifndef NO_DEC
	if ( base == 10 )
		/*		convert from decimal		*/
		/*		integer part		*/
	{ while ( ( bn_n_( * sg ) ) < 10 )			/* step down digits */
		{ if ( bn_dec_in_ip ( * sg , & sc ) )	     	/* insert digits */
			return ( 1 ) ; ++ sg ; }
		if ( bn_decip_end ( & sc ) ) return ( 1 ) ;	/* convert */
#if FRAC
		/*		prepare fraction part		*/
		if ( * sg == '.' )			/* if found frac */
		{ ++ sg ;
			while ( bn_n_( * sg ) < 10 ) { ++ j ; ++ sg ; }
			-- sg ;			/* at end */
			/*		fraction part conversion		*/
			while ( * sg != '.' )
			{
#if BIG_DAJ == 1
				rem = rem |					/* keep underflow */
#endif
					bndivul ( ( ublong ) * ( sg -- ) - '0' ,
							L2( 10 ) , x , x , S_FRAC ) ; }
#if BIG_DAJ == 1
				bnadd_ul ( ( blong ) ( asc_inc && rem && ( j == FRAC_CHAR ) ) , x , 0 ) ;
#endif
		} 						/* end '.' */
#endif
	}							/* end decimal */
	else
#endif
		/*		convert from hex		*/
	{ if ( ! ( * sg == '.' ) )
		{ s2 = sg ;
			while ( bn_n_( * s2 ) < 16 ) { ++ s2 ; ++ pos ; } /* digits in integer */
			if ( pos > ( ( S_PREC - S_FRAC ) * E_SZ( 4 , 8 ) ) ) return ( 1 ) ;
			x2 = x2 + BN_FRAC( S_FRAC + ) ( ( pos - 1 ) >> E_SZ( 2 , 3 ) ) ;
			/*		hex integer part		*/
			while ( pos -- )					/* down integer */
			{ * x2 = * x2 |
				( bn_n_( * sg ) << ( ( pos & E_SZ( 3 , 7 ) ) * 4 ) ) ;
				++ sg ;
				if ( ! ( pos & E_SZ( 3 , 7 ) ) ) -- x2 ; } }
#if FRAC
		if ( * sg == '.' )					/* maybe end */
			/*		hex fraction part		*/
		{ ++ sg ; x2 = x + S_FRAC - 1 ; pos = S_FRAC * E_SZ( 4 , 8 ) ;
			while ( ( ( n = bn_n_( * sg ) ) < 16 ) &&
					( ( pos >> E_SZ( 2 , 3 ) ) > - 1 ) )
			{  -- pos ; * x2 = * x2 | ( n << ( ( pos & E_SZ( 3 , 7 ) ) * 4 ) ) ;
				++ sg ;
				if ( ! ( pos & E_SZ( 3 , 7 ) ) ) -- x2 ; } }
#endif
	}
	BN_SIG( x [ S_PREC ] = sig ; mssb ( x ) ; )
		return ( 0 ) ; }			/* sc AT EM = 0 IS NOT USED */

		/** output to a stream				*/

		/**		  bn_out
		  if hex /	\ if decimal
		  /	 \
		  bn_m_00	  readyx		these initialise as needed
		  makes buffer	  |
		  \	 /
		  bn_s_out			does the work	*/		

		R_D big_out ( int name_len , char * sg , bele * x , FILE * sm , int mark ,
				/* ^^^		ch length     string     big number      stream   begin mark */
				int b_end , int group , unsigned line_len , blong chip ,
				/* ^^^	  end mark       group	     line length   integer-part characters  */
#if FRAC
				int fmode ,			/* mode for chfp */
				blong chfp ,		/* fraction-part characters */
#endif
				int leadz ,	 int base )
		/* ^^^	     to replace leading 0   base		*/

{ struct bnout ctrl ;
	ctrl.nam_len = name_len ; ctrl.sg = sg ;
	ctrl.mark = mark ; ctrl.x = x ; ctrl.b_end = b_end ;
	ctrl.group = ( blong ) group ; ctrl.line_len = ( blong ) line_len ;
	ctrl.base = base ;
	ctrl.sm = sm ; BN_DEC( ctrl.achip = ) ctrl.chip = chip ;
	ctrl.leadz = ctrl.lead1 = leadz ;
#if FRAC
	ctrl.ofm = fmode & 1 ; ctrl.fpnl = fmode & 2 ; ctrl.frnd = ! ( fmode & 4 ) ;
	ctrl.chfp = chfp ;
#endif
#if ERROR_MODE
	if ( ( sm == NULL ) ? 1 :
			( BN_DEC( base == 10 ? bn_m_00 ( & ctrl ) : ) bn_readyx ( & ctrl ) ) )
		BNE_FUN( "writing ascii " ) ; BN0 ; }
#else
if ( sm != NULL )
	BN_DEC( if ( base == 10 ) bn_m_00 ( & ctrl ) ; else )
	bn_readyx ( & ctrl ) ; }
#endif

	/**	bn_readyx and bn_m_00 have to prepare the control values in the
	  struct ctrl
	  arrival			      use
	  ctrl->chip	i-p request		      i-p characters
	  ctrl->ofm	f-p mode
0 : ctrl->chfp as a dock
1 : ctrl->chfp as length
ctrl-chfp	f-p length request	      neg of f-p characters
ctrl->n					      number of i-p n000
ctrl->achip	dec i-p min  places	      i-p places
at arrival out of range values arriving produce the natural length

has to set
ctrl->n					      the top hex digit
lead1					      lead ch for o-f position
*/
BN_E bn_readyx ( struct bnout * ctrl )
{ blong len ; BN_FRAC( blong add_8 ; int pl ; BN_ERR( int bnerr ; ) )
	ctrl->fcall = bn2x ;

	/*	set ctrl->achip		*/
	ctrl->lead1 = ( int ) ( ( blong ) ctrl->chip == BNXICH ) ;
	/* horrible patch, elegance has competitors */
	if ( ! ctrl->chip || ( ( ublong ) ctrl->chip >= BNXICH ) )
		ctrl->achip = BNXICH ;
	/* set achip for integer-part natural length */
	else
		if ( ctrl->chip < ( len = ( ntop ( ctrl->x ) / L2( 4 ) + L2( 1 ) ) ) )
			/* characters in i-p , not counting o-f */
			ctrl->achip = len ;
#if FRAC
	/*	set ctrl->chfp		*/
	if ( ( ublong ) ctrl->chfp > ( S_FRAC * BNXPE ) )
	{ ctrl->ofm = 0 ; ctrl->chfp = L2( 0 ) ; }
	ctrl->chfp = ctrl->ofm ? - ctrl->chfp :
		ctrl->chfp - S_FRAC * BNXPE ;		/* set chfp to minus length */
	/*	rounding and possible consequent o-f		*/
	if ( ( ctrl->chfp + S_FRAC * BNXPE ) )    /* if chfp + cut */
	{ ctrl->ofm = bnadd_ul ( add_8 = ( blong ) ( L2( 8 ) <<	/* ofm is o-f */
				( ( ( S_FRAC * BNXPE +
					  ctrl->chfp - L2( 1 ) ) & E_SZ( 3L , L2( 7 ) ) ) * L2( 4 ) ) )
			* ctrl->frnd ,			  /* add_8 is the 8 positioned in int */
			ctrl->x ,
			pl = ( unsigned int ) ( ( blong ) ( S_FRAC * BNXPE +
					ctrl->chfp - L2( 1 ) ) >> E_SZ( 2L , L2( 3 ) ) ) ) ;
	ctrl->lead1 = ctrl->lead1 ? ' ' : ctrl->leadz ; }
	else { ctrl->ofm = 0 ; add_8 = L2( 0 ) ; ctrl->lead1 = ctrl->leadz ; }
	/* ofm to bn_s_out, add_8 to below */
	ctrl->achip += ( ( add_8 > 0 ) && ( ctrl->achip == BNXICH ) ) * ctrl->frnd ;
	/* 1 more if add_8 and full length i-p */
#endif
	ctrl->n = ctrl->achip
#if FRAC
		+ ( blong ) S_FRAC * E_SZ( 4L , L2( 8 ) ) ;
	BN_ERR( bnerr = ) bn_s_out ( ctrl ) ;
	if ( add_8 ) bns_lipos ( add_8 , ctrl->x , pl ) ;		/* restore */
	BN_ERR( return ( bnerr ) ) ; }
#else
	;					/* not an accident */
BN_ERR( return ) ( bn_s_out ( ctrl ) ) ; }
#endif

#ifndef NO_DEC
#define BNZ( A ) (int) ((ublong) BNASZ( A )/E_SZ(4L,L2(9))+L2(1))
#define BN00MX BNZ( BNHOP )
#if ( defined USE_HEAP && defined SOFT_SIZE ) || ( PREC > BNHOP )
#ifdef SOFT_SIZE
#define BN_UP( A ) if( A ++ == ctrl->n00top && (S_PREC>BNHOP)) A = ctrl->n002
#define BN_DO( A ) if( A -- == ctrl->n002 && (S_PREC>BNHOP)) A = ctrl->n00top
#else
#define BN_UP( A ) if ( A ++ == ctrl->n00top ) A = ctrl->n002
#define BN_DO( A ) if ( A -- == ctrl->n002 ) A = ctrl->n00top
#endif
#else
#define BN_UP( A ) ++ A
#define BN_DO( A ) -- A
#endif

/**	for decimal output
  makes a buffer and, if needed an overflow buffer
  load this with clusters of decimal places
  then call bn_s_out

  written as a number, least significant to the right :

  [ ] ... [ ] . [ ] ... [ ]
  ^	       ^
  numb	      n00

  when the n00s have to be broken because there are too many
  numb pointing to the 0 int place may be in either array :

  [ ] ... [ ]		[ ] ... [ ]
  ^		 ^
  n002	    n00top	n00

  test pointer going
  up against n00top after incrementing
  down against n002 before decrementing			*/

BN_E bn_m_00 ( struct bnout * ctrl )
{ BN_FRAC( bele * n00fp ; )
	unsigned m ; blong len ;
	int bn2 ;
#if FRAC
	bele * numb , noddy = 0 ;
#endif
	BN_HEAP( BN_ERR( int bnerr = 1 ; ) )		/* set to zero by success */
#ifndef USE_HEAP
		/*	take stack	*/
#ifndef SOFT_SIZE
		bele numb0 [ ( ( PREC - FRAC ) > FRAC ? PREC - FRAC : FRAC ) ] ;
	/* fraction-part , integer-part */
#else
	big_n numb0 ;
#endif
#if ( PREC > BNHOP )
	bele n00 [ BNZ( BNHOP ) ] , n002 [ BNZ( PREC - BNHOP ) + 1 ] ;
	ctrl->n002 = n002 ;
#else
	bele n00 [ BNZ( PREC ) + 1 ] ;	/* + 1 for 2 roundings up */
#endif
	BN_FRAC( numb = numb0 ; )
#else
		bele * n00 , * numb0 ;
#endif
	if ( ! ctrl->chip || ( ( ublong ) ctrl->chip > INT_CHAR ) )
		len = ctrl->achip = INT_CHAR ;  	/* natural length */
	else
		len = BNBSZ( bn_msx ( & ctrl->x [ S_FRAC ] , S_PREC - S_FRAC ) ) ;
	ctrl->n = ( len + E_SZ( 3L , L2( 8 ) ) ) / E_SZ( 4L , L2( 9 ) ) ;
	if ( len  > ctrl->achip ) ctrl->achip = len ;
#if FRAC
	/* ofm determines how chfp will be used, see above
	   then when that is resolved, signals f-p to o/p shorter than naturally
	   chfp of 0 or outside the natural length gives the natural length
	   chfp is used as a negative */
#ifdef SOFT_SIZE
	if ( ! S_FRAC ) { ctrl->ofm = 1 ; ctrl->chfp = L2( 0 ) ; }
#endif
	if ( ( ublong ) ctrl->chfp > FRAC_CHAR )
	{ ctrl->ofm = 0 ; ctrl->chfp = L2( 0 ) ; }		/* natural length */
	ctrl->chfp = ctrl->ofm ? - ctrl->chfp : ctrl->chfp - FRAC_CHAR ;
	/*		control	  - given	       dok	*/
	ctrl->ofm = ( ( - ctrl->chfp ) != FRAC_CHAR ) ;
	/* reuse */
#endif
#ifdef USE_HEAP
	/*	take heap memory	*/


	BN_FRAC( numb = ) numb0 = ( bele * ) bn_h (
#if FRAC
			( S_PREC - S_FRAC ) > S_FRAC ? S_PREC - S_FRAC : S_FRAC
#else
			S_PREC
#endif
			, BY_P_E ) ;

	n00 = ( bele * )
		bn_h ( S_PREC > BNHOP ? BN00MX : BNZ( S_PREC ) + 1 , BY_P_E ) ;
	/*		initialise n00ip, save point		*/
#if ( defined USE_HEAP && defined SOFT_SIZE ) || ( PREC > BNHOP )
	ctrl->n00top = n00 + BNZ( S_PREC > BNHOP ? BNHOP : s_prec ) - 1 ;
#endif
#if defined USE_HEAP && ( ( defined SOFT_SIZE ) || ( PREC > BNHOP ) )
	ctrl->n002 = ( bele * ) bn_h (
#ifdef SOFT_SIZE
			( S_PREC <= BNHOP ) ? 1 :
#endif
			BNZ( S_PREC - BNHOP ) + 1 , BY_P_E ) ;
#endif
#endif
	ctrl->n00ip =					/* n00 place 0 */
#if FRAC
		n00fp =
#if defined USE_HEAP && ( ( defined SOFT_SIZE ) || ( PREC > BNHOP ) )
		( S_FRAC > BNHOP ) ? ctrl->n002 + BNZ( S_FRAC - BNHOP ) :
#endif
		BNZ( S_FRAC ) +		/* n00fp start at the unit place */
#endif
		n00 ;
	/* these fills can be slightly accelerated, is it worth it ? */
	/*		fill n00s, integer-part			*/
	bnclen ( & ctrl->x [ S_FRAC ] , numb0 , m = S_PREC - S_FRAC ) ;
	ctrl->n = ( ( blong ) ( ctrl->achip +
				E_SZ( 3L , L2( 8 ) ) ) / E_SZ( 4L , L2( 9 ) ) ) ;
	while ( ctrl->n -- )
	{ * ctrl->n00ip = bndivul ( L2( 0 ) , E_SZ( 10000UL , L2( 1000000000U ) ) ,
			numb0 , numb0 , m ) ;
	m -= ( ! numb0 [ m - 1 ] ) && ( m > 0 ) ;		/* shorten i-p */
	BN_UP( ctrl->n00ip ) ; }
#if FRAC
	/*		fill n00s, fraction-part		*/
	bnclen ( ctrl->x , numb , bn2 = S_FRAC ) ;		/* big fraction-part */
	ctrl->n =				/* number of n00s to load in f-p */
		( - ctrl->chfp + E_SZ( 3L , L2( 8 ) ) + ctrl->ofm )
		/ E_SZ( 4L , L2( 9 ) ) ;	/* number of f-p n00s to fill */
	while ( ctrl->n -- )
	{ BN_DO( n00fp ) ;
		* n00fp = bnmulul ( E_SZ( 10000UL , L2( 1000000000U ) ) ,
				numb , numb , bn2 ) ;
		if ( ( ! * numb ) && bn2 ) { -- bn2 ; ++ numb ; } }	/* shorten f-p */
	/*  n00fp points to last n00 filled */
	/* dock tail and round for dock or BIG_DAJ */
	/* will hold 10 * 10 ^ N or 5 * 10 ^ N , to add */
	m =	( ctrl->ofm ? E_SZ( 5000UL , L2( 500000000U ) ) : (
#if BIG_DAJ == 2
				bn_msx ( numb , bn2 ) != - L2( 1 ) ?
				E_SZ( 10000UL , L2( 1000000000U ) ) :
#endif
				L2( 0 ) ) ) * ctrl->frnd ;
	noddy = ( int ) ( ( blong )
			( - L2( 0 ) - ctrl->chfp ) % E_SZ( 4L , L2( 9 ) ) ) ;
	while ( noddy -- ) m /= 10 ;			/* position the 5 */
	/* add and test for excess + 5 or + 1 at required place */
	if ( ( * n00fp += m ) > E_SZ( 9999U , 999999999U ) )   /* but cast 64 bit */
	{ * ( n00fp ++ ) = 0 ;
		while ( ++ * n00fp == E_SZ( 10000 , 1000000000 ) )
		{ * n00fp = 0 ;					/* carry up the n000s */
			BN_UP( n00fp ) ; } }				/* will never o-f */
#else
#undef bn2
	/* V end if FRAC */
#endif
	/*	prepare for function bn210		*/
	-- ctrl->n00ip ;
	ctrl->bebuff = * ctrl->n00ip ;
	if ( ( m = ( int ) ( ( ctrl->n = ctrl->achip ) % E_SZ( 4 , 9 ) ) ) != 0 )
	{ m = E_SZ( 4 , 9 ) - m ;
		while ( m -- ) ctrl->bebuff *= 10 ; }
	/*	if needed by ctrl->chip , lose top 0 */
	if ( ctrl->chip && ( ctrl->achip > ctrl->chip ) &&
			! ( ctrl->bebuff / E_SZ( ( bele ) 1000 , 100000000 ) ) )
	{ -- ctrl->achip ; bn210 ( ctrl ) ; }
	/* set call */
	ctrl->fcall = bn210 ;
	/*			send				*/
	BN_ERR( BN_NHEAP( return ) BN_HEAP( bnerr = ) )
		bn_s_out ( ctrl ) ;
#ifdef USE_HEAP
#if ( defined SOFT_SIZE ) || ( PREC > BNHOP )
	FREE_BN_( ctrl->n002 ) ;
#endif
	FREE_BN_( n00 ) ; FREE_BN_( numb0 ) ;
	BN_ERR( return ( bnerr ) ) ;
#endif
}
#endif

BN_E bn_s_out ( struct bnout * ctrl )
{ BN_ERR( int bnerr = 0 ; ) unsigned u , program ;
	ublong lu , pos ;
	int i = 0 , activ = '0' , tokens ;
	char mark_sg [ 3 ] = "  " ;
#if SIGNED
	mssb ( ctrl->x ) ;					/* mend -0 */
#endif
	if ( ctrl->mark == 10 ) mark_sg [ 1 ] = 0 ;		/* equals one space */
	else mark_sg [ 0 ] = ctrl->mark ;
	/* 0 or 10 give no string length */
	/*		line length and group		*/
	ctrl->line_len = ctrl->line_len > L2( 2 ) ?
		ctrl->line_len - L2( 2 ) : ( unsigned ) ( - L2( 1 ) ) ;

	if ( ctrl->group >= ctrl->line_len ) ctrl->group = ctrl->line_len ;
	/*		print label		*/
	tokens = ( ( ( ctrl->mark && ( ctrl->mark != 10 ) ) ? 2 : 1 )
			+ ( ctrl->base ? 0 : 3 ) + SIGNED + SIGNED ) ;
	/*	send label		*/
	if ( ctrl->sg )				/* NULL gives no label */
	{ while ( * ctrl->sg )			/* step through sg */
		{ if ( ( * ctrl->sg > 31 ) BN_NERR )	/* ignore non printable */
			{ BN_ERR( bnerr = ) ( BNEEOF putc( * ctrl->sg , ctrl->sm ) ) ; /* o/p */
				++ i ; }
			++ ctrl->sg ; } }
	/*		position cursor and send tokens		*/
	if ( ( ctrl->mark != 10 ) && ( i <= ctrl->nam_len ) )
	{ pos = ctrl->nam_len + ( ctrl->nam_len > 0 ) + tokens ;
		i += tokens ;
		program = 0451 +			/* adjust , pad , tokens */
			! ctrl->group ; }	/* set for group = 0 */
	else
	{ pos = ( i = tokens ) + 1 ;
		program = 04531 - ! ctrl->group ; }	/* adjust , \n , pad , tokens */

	while ( program )
	{ switch ( program & 7 )
		{ case 0 :				/* group == 0 , mark == 10 */
			program = 05437 ;			/* >> , \n , tokens , */
			break ;
			case 1 :				/* calculate adjustment */
			lu = ctrl->achip % ctrl->group ;
			if ( ! lu ) lu = ctrl->group ;
			if ( pos >
					( u = ( int ) ( ( blong ) ctrl->line_len -
									( ctrl->line_len + L2( 1 ) ) % ( ctrl->group + L2( 1 ) ) - lu ) ) )
			{ pos = i = tokens ;
				program = 04537 ;		/* >> , \n , pad , tokens */
				if ( pos > u )
				{ pos = i = 0 ;
					program = ( ctrl->mark == 10 ) ?
						053437 :		/* >> , \n , tokens , \n , pad */
						05347 ; } }		/* >> , tokens , \n , pad */
			if ( ( u = ( ctrl->group -
							( int ) ( ( blong ) ctrl->achip % ctrl->group + pos ) %
							( ctrl->group + 1 ) ) % ( ctrl->group + 1 ) ) != ctrl->group )
				pos += u ;
			else
			{ if ( pos ) -- pos ;
				else program = ( ctrl->mark == 10 ) ?
					0437 :			/* >> , \n , tokens */
						047 ; }			/* >> , tokens */
			break ;
			case 2 :				/* group == 0 */
			pos = ctrl->nam_len + tokens ;
			program = 0457 ; 			/* >> , pad , tokens */
			break ;
			case 3 :				/* newline */
#ifdef CLEOL
			if ( ctrl->sm == stdout ) CLEOL ;
#endif
			BN_ERR( if ( ! bnerr ) bnerr = ) ( BNEEOF putc( '\n' , ctrl->sm ) ) ;
#ifdef CLEOL
			if ( ctrl->sm == stdout ) CLEOL ;
#endif
			break ;
			case 4 :				/* put tokens */
			BN_ERR( if ( ! bnerr ) bnerr = )
				( BNEEOF fprintf( ctrl->sm , "%s"		/* mark */
#if SIGNED
								  "%c "						/* sign */
#endif
								  "%s" , mark_sg					/* 0x */
#if SIGNED
								  , ctrl->x [ S_PREC ] ? '-' : '+'
#endif
								  , ctrl->base ? "" : X_x ) ) ;		/* allow 0X */
#ifdef CLEOL
			if ( ctrl->sm == stdout ) CLEOL ;
#endif
			break ;
			case 5 :				/* pad before tokens */
			i = pos - i + 1 ;
			while ( i -- BN_NERR )
				BN_ERR( bnerr = ) ( BNEEOF putc( ' ' , ctrl->sm ) ) ;
			break ; }
		program >>= 3 ; } ;
	/*		send the body			*/
	while ( ( ctrl->achip BN_FRAC( > ctrl->chfp ) ) BN_NERR )
		/* character due && ! error */
	{ ++ pos ;				/* this is this character increment */
		if ( ( ctrl->group && ( blong ) ! ( ctrl->achip % ctrl->group ) ) ||
				! ctrl->achip || ( pos > ctrl->line_len ) )
			/*		group break		*/
		{ ++ pos ;
			if ( ( ( pos >= ( ctrl->line_len - ctrl->group - ! ctrl->group ) ) &&
						ctrl->line_len )
#if FRAC
					|| ( ! ctrl->achip && ctrl->fpnl )
#endif
			   )						/* line end */
			{ BN_ERR( if ( ! bnerr ) bnerr = )
				( BNEEOF putc( '\n' , ctrl->sm ) ) ;
#ifdef CLEOL
				if ( ctrl->sm == stdout ) CLEOL ;
#endif
				pos = 1 ; }					/* turn & then space */
#if FRAC
			BN_ERR( if ( ! bnerr ) bnerr = )
				( BNEEOF putc( ctrl->achip ? ' ' : '.' , ctrl->sm ) ) ;
#else
			BN_ERR( if ( ! bnerr ) bnerr = )
				( BNEEOF putc( ' ' , ctrl->sm ) ) ;
#endif
		}							/* end group break */
		BN_ERR( if ( ! bnerr ) )
		{ -- ctrl->achip ;
			/* get the character from the function assigned to ctrl->fcall */
			u = ( * ctrl->fcall ) ( ctrl ) ;		/* 000 suppress */
			activ |= ( u | ( ctrl->achip == 0 ) ) ;
			BN_ERR( bnerr = )
				( BNEEOF putc( activ == '0' ? ctrl->lead1 : u , ctrl->sm ) ) ;
			ctrl->lead1 = ctrl->leadz ;
		} }				/* end of : while , if ! bnerr */
	/*	terminator, \n, error call, return		*/
	if ( ctrl->b_end && ( ctrl->b_end != 13 ) BN_NERR ) BN_ERR( bnerr = )
		( BNEEOF putc( ( char ) ctrl->b_end , ctrl->sm ) ) ;
#ifdef CLEOL
	if ( ctrl->sm == stdout ) CLEOL ;
#endif
	/*	final newline		*/
#ifndef BNNONL
	BN_ERR( if ( ! bnerr ) bnerr = ) ( BNEEOF ( putc( '\n' , ctrl->sm ) ) ) ;
#endif
	return BN_ERR( ( bnerr ) ) ; }

#ifndef NO_DEC
int bn210 ( struct bnout * ctrl )
{ int u ;
	u = ( int ) ( ( bele ) ctrl->bebuff / E_SZ( 1000 , 100000000 ) ) ;  /* take */
	ctrl->bebuff = ( ctrl->bebuff - E_SZ( 1000 , 100000000 ) * u ) * 10 ;
	if ( ! ( -- ctrl->n % E_SZ( 4L , L2( 9 ) ) ) )
	{ BN_DO( ctrl->n00ip ) ;
		ctrl->bebuff = * ctrl->n00ip ; }
	return ( u + '0' ) ; }

#undef BN00MX
#undef NSZ
#undef BNHOP
#undef BN_UP
#undef BN_DO
#endif

	/* bn2x decrements ctrl->n and returns the indicated ascii hex */
int bn2x ( struct bnout * ctrl )
{ int ich ;
	-- ctrl->n ;
#if FRAC
	if ( ctrl->n == ( ( blong ) ( S_PREC ) * E_SZ( 4 , 8 ) ) )
		return ( ctrl->ofm ? '1' : ' ' ) ;
#endif
	ich = ( ( unsigned )
			ctrl->x [ ( int ) ( ( blong ) ctrl->n >> E_SZ( 2L , L2( 3 ) ) ) ] >>
			( ( ctrl->n & E_SZ( 3L , L2( 7 ) ) ) * L2( 4 ) ) ) & L2( 15 ) ;
	return ( ich + ( ich > 9 ? ( X_ten - 10 ) : '0' ) ) ; }

	/** output a big number as bits

	  bit_show		outputs to the console
	  |
	  bits_out		to any stream
	  |
	  bn_bits2sm		one element to stream		*/

void bit_show ( bele * w )				/* display w as bits */
{ bits_out ( w , S_LEN , stdout ) ; }

R_D bits_out ( bele * w , int m , FILE * sm )
{ BN_ERR( int bnerr = 0 ; ) int mm ;
	mm = m ;
	while ( m -- BN_ERR( && ! bnerr ) )			/* all elements */
	{ BN_ERR( bnerr = ) ( BNEEOF fprintf( sm ,
				BN_FRAC( m == ( S_FRAC - 1 ) ? BIG_B_POINT : ) "   " ) ) ;
	BN_ERR( if ( ! bnerr ) bnerr = ) bn_bits2sm ( w [ m ] , sm ) ;
	if ( ! m || ! ( ( mm - m ) % ( ( LINE_LEN - 2 ) / E_SZ( 19 , 35 ) ) ) )
		/* if end or EOL */
	{
#ifdef CLEOL
		if ( sm == stdout ) CLEOL ;
#endif
		BN_ERR( bnerr = ) ( BNEEOF putc( '\n' , sm ) ) ; } }  /* end all loop */
	BN_ERR( if ( bnerr ) ) BNE_FUN( "in bits_out" ) ;
	BN0 ; }

BN_E bn_bits2sm ( bele be , FILE * sm )
{ unsigned o = E_SZ( 0x8000 , 0x80000000 ) ;
	do
	{ if ( putc( ( be & o ) ? '1' : BIG_B_0 , sm ) == EOF ) BNE1 ; }
	while ( ( o = o >> 1 ) != 0 ) ;
	BNE0 ; }

	/**	error function	*/

#if ( ( ERROR_MODE > 1 ) && ( ERROR_MODE < 64 ) )
void error_function ( char * sg )
{
#ifndef NO_BELL
	putchar( '\a' ) ;
#endif

	fprintf( ERR_SM , "\nerror : %s\n"
#if ! ( ERROR_MODE & 8 )
			"press a key "
#endif
			, sg ) ;
#if ! ( ERROR_MODE & 8 )
#if ( ERROR_MODE & 63 ) == 2
	fprintf( ERR_SM ,
#ifdef GET_CH
			"to continue\n" ) ;
	GET_CH ;
#else
	"to continue, and then press cr or enter\n" ) ;
	fflush ( stdin ) ; getchar ( ) ; fflush ( stdin ) ;
#endif
#elif ( ERROR_MODE & 63 ) == 3
	fprintf( ERR_SM ,
#ifdef GET_CHE
			"or press q to quit\n" ) ;
	if ( ( GET_CH | 32 ) != 'q' ) return ;
#else
	"or enter q to quit\n" ) ;
	fflush ( stdin ) ;
	if ( ( getchar ( ) | 32 ) != 'q' ) return ;
#endif
#elif ( ERROR_MODE & 63 ) == 4
	fprintf( ERR_SM , "to quit\n" ) ;
#ifdef GET_CH
	GET_CH ;
#else
	fflush ( stdin ) ; getchar ( ) ; fflush ( stdin ) ;
#endif
#endif
#endif
#if ( ERROR_MODE & 7 ) > 2
	fprintf( ERR_SM , "process inactive\n" ) ;
#ifdef ICLOSE
	ICLOSE ;
#endif
	exit ERR_EX ;
#endif
}
#endif

#ifdef USE_HEAP
#ifndef BN_TAKE_GOT
#define BN_TAKE_GOT

/**	take heap

  argument i of j bytes, return to pointer	*/

void * bn_h ( int i , int j )
{ void * z ;
	if ( ( z = ( void * ) calloc ( i , j ) ) == NULL )
	{ fprintf( ERR_SM , "\tno heap memory left\n" ) ;
#ifdef ICLOSE
		ICLOSE ;
#endif
		exit ERR_EX ; }
	return ( z ) ; }

#endif
#endif

#endif

