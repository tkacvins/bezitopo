/******************************************************/
/*                                                    */
/* measure.cpp - measuring units                      */
/*                                                    */
/******************************************************/


#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include "measure.h"
using namespace std;

const struct cf
  {int unitp;
   double factor;
   } cfactors[]=
{0,		1,			// unknown unit
 MILLIMETER,	0.001,			// mm
 MICROMETER,	0.000001,		// μm
 KILOMETER,	1000,			// km
 METER,		1,			// m
 INCH,		0.0254,			// in
 FOOT,		0.3048,			// ft
 SURVEYFOOT,	0.3048006096012192,	// 
 GRAM,		0.001,			// g
 KILOGRAM,	1.0,			// kg
 POUND,		0.45359237,		// lb
 MILLIPOUND,	0.00045359237,		// pound per thousand
 OUNCE,		0.028349523125,		// oz
 TROYOUNCE,	0.0311034768,		// troy ounce
 HOUR,		3600,			// hour
 KGPERL,	1000.0,			// kg/l
 LBPERIN3,	27679.90471020312,	// lb/in3
 PERKG,		1.0,			// $/kg
 PERPOUND,	2.2046226218487759,	// $/lb
 PERMETER,	1.0,			// $/m
 PERFOOT,	3.2808398950131233,	// $/ft
 PERLITER,	1000,			// /L
 EACH,		1,			// $/piece
 PERHUNDRED,	0.01,			// $/hundred pieces
 PERTHOUSAND,	0.001,			// $/thousand pieces
 PERLOT,	1,			// $/lot
 PERSET,	1,			// $/set
 ITEM,		1,
 THOUSAND,	1000,
 LOT,		1,
 SET,		1,
 PERMONTH,	0.38026486208173715e-6,
 PERYEAR,	31.688738506811427e-9,
 PERHOUR,	0.00027777777777777777,	// $/hour
 MILLILITER,	0.000001,
 IN3,		0.000016387064,
 };
#define nunits (sizeof(cfactors)/sizeof(struct cf))
struct symbol
  {int unitp;
   char symb[12];
   } symbols[]=
/* The first symbol is the canonical one, if there is one. */
{MILLIMETER,	"mm",
 MICROMETER,	"µm", //0000b5 00006d
 MICROMETER,	"μm", //0003bc 00006d
 MICROMETER,	"um",
 INCH,		"in",
 INCH,		"\"",
 INCH,		"INCHES",
 INCH,		"IN.",
 FOOT,		"ft",
 FOOT,		"'",
 FOOT,		"FEET", /* occurs in QUANT.FIL */
 FOOT,		"FT.",
 FOOT,		"FT",
 FOOT,		"FT. LGTHS",
 FOOT,		"FT. LGTHS.",
 FOOT,		"FT. LGTH.",
 FOOT,		"FT. LGTH",
 GRAM,		"g",
 KILOGRAM,	"kg",
 POUND,		"lb",
 POUND,		"LB",
 POUND,		"LB.",
 POUND,		"Pound",
 POUND,		"Pounds",
 POUND,		"#",
 MILLIPOUND,	"lb/1000",
 KGPERL,	"kg/l",
 KGPERL,	"kg/L",
 KGPERL,	"g/ml",
 KGPERL,	"g/mL",
 LBPERIN3,      "lb/in³",
 EACH,		"ea",
 EACH,		"ea.",
 EACH,		"EACH",
 EACH,		"EA.",
 EACH,		"EEA.",
 EACH,		"EA.*",
 EACH,		"EA..",
 EACH,		"EA*",
 EACH,		"EA.^",
 EACH,		"EA. *",
 EACH,		"EA",
 PERHUNDRED,	"/C",
 PERTHOUSAND,	"/M",
 PERLOT,	"/LOT",
 PERLOT,	"/LT",
 PERLOT,	"/LO",
 PERSET,	"/SET",
 PERSET,	"/SE",
 PERPOUND,	"/lb",
 PERPOUND,	"/LB.",
 PERPOUND,	"/LB",
 MILLILITER,	"ml",
 IN3,		"in³",
 PERLITER,	"/L", /* placeholder for chopped-off /LB or /LOT */
 LOT,		"LOT",
 LOT,		"Lot",
 LOT,		"Lots",
 SET,		"SET",
 SET,		"SETS",
 ITEM,		"",
 ITEM,		"PCS",
 ITEM,		"PCS.",
 ITEM,		"Piece",
 ITEM,		"Pieces",
 ITEM,		"PARTS",
 THOUSAND,	"M", /* will be changed to k in history program */
 THOUSAND,	"M PARTS",
 };
#define nsymbols (sizeof(symbols)/sizeof(struct symbol))

int msystem=1; /* 0 or 1: metric. 2 or 3: US customary. 1 or 3: or as entered. */
int substtable_met_small[]=
{MILLIMETER+DEC3,
 GRAM+DEC3,
 KGPERL+DEC3,
 MILLILITER+DEC3,
 ITEM+DEC3};
int substtable_met_big[]=
{METER+DEC3,
 KILOGRAM+DEC3,
 KGPERL+DEC3,
 MILLILITER+DEC0,
 ITEM+DEC3};
int substtable_usc_small[]=
{INCH+DEC3,
 MILLIPOUND+DEC3,
 LBPERIN3+DEC3,
 IN3+DEC3,
 ITEM+DEC3};
int substtable_usc_big[]=
{FOOT+DEC3,
 POUND+DEC3,
 LBPERIN3+DEC3,
 IN3+DEC1,
 ITEM+DEC3};
#define substtable_size (sizeof(substtable_met_big)/sizeof(int))
int length_unit=METER;
double length_factor=1;

int fequal(double a, double b)
/* Returns true if they are equal within reasonable precision. */
{return (fabs(a-b)<fabs(a+b)*1e-13);
 }

int is_int(double a)
{return fequal(a,rint(a));
 }

double cfactor(int unitp)
{int i;
 for (i=0;i<nunits;i++)
     if (same_unit(unitp,cfactors[i].unitp))
        return cfactors[i].factor;
 //fprintf(stdout,"Conversion factor for %x missing!\n",unitp);
 return 0/0;
 }

void set_length_unit(int unitp)
{if (compatible_units(unitp,METER) && cfactor(unitp)>0)
    {length_unit=unitp;
     length_factor=cfactor(unitp);
     }
 }

char *symbol(int unitp)
{int i;
 for (i=0;i<nsymbols;i++)
     if (same_unit(unitp,symbols[i].unitp))
        return symbols[i].symb;
 return "unk";
 }

int is_exact(double measurement, unsigned int unitp,int is_toler)
/* Checks whether the given measurement is exact in that unit.
   If the measurement is a tolerance and is divisible by 127, returns false;
   this means that a tolerance in inches is being checked in millimeters. */
{unsigned int base,exp,i;
 double factor,m;
 base=10;
 exp=unitp&31;
 if (exp>15)
    {exp-=16;
     base=2;
     }
 factor=cfactor(unitp);
 for (i=0;i<exp;i++)
     factor/=base;
 m=measurement/factor;
 return (is_int(m) && !(is_toler && is_int(m/127.0) && m!=0));
 }

char *trim(char *str)
/* Removes spaces from both ends of a string in place. */
{char *pos,*spos;
 for (pos=spos=str;*pos;pos++)
     if (!isspace(*pos))
        spos=pos+1;
 *spos=0;
 for (pos=str;isspace(*pos);pos++);
 memmove(str,pos,strlen(pos)+1);
 return(str);
 }

char *collapse(char *str)
/* Collapses multiple spaces into single spaces. */
{char *pos,*spos;
 for (pos=spos=str;*pos;pos++)
     if (!isspace(*pos) || !isspace(pos[1]))
        *spos++=*pos;
 *spos=0;
 return(str);
 }

double precision(int unitp)
/* Returns the precision (>=1) of unitp. Base codes 3-f are returned as 0
   and must be handled specially by the formatter.
   00 1
   01 10
   02 100
   ...
   0f 1000000000000000
   10 1
   11 2
   12 4
   ...
   1f 32768
   20 1
   21 60
   22 3600
   22 216000
   23 10
   24 600   e.g. 63°26.1′
   25 36000
   ...
   2f 216000000
   30-3f Used for mixed units.
   40-ff Undefined.
   */
{
  double base,p;
  int exp,basecode,i;
  exp=unitp&0xf;
  basecode=unitp&0xf0;
  switch (basecode)
  {
    case 0:
      base=10;
      break;
    case 16:
      base=2;
      break;
    case 32:
      base=60;
      break;
    default:
      base=0;
  }
  if (base==60)
  {
    for (p=1,i=0;i<(exp&3);i++)
      p*=base;
    for (i=0;i<(exp&12);i+=4)
      p*=10;
  }
  else
    for (p=1,i=0;i<exp;i++)
      p*=base;
  return p;
}

int moreprecise(int unitp1,int unitp2)
/* Given two unitp codes, returns the more precise. If one of them has no
   conversion factor, or they are of different quantities, it still returns
   one of them, but which one may not make sense. */
{double factor1,factor2;
 factor1=cfactor(unitp1)/precision(unitp1);
 factor2=cfactor(unitp2)/precision(unitp2);
 if (factor1<factor2)
    return unitp1;
 else
    return unitp2;
 }

void switch_system()
{msystem=(msystem+1)%4;
 }

int subst_unit(int unitp)
/* Substitutes a unit from substtable, depending on the currently selected measuring system. */
{int *table,i;
 if (((msystem&1)==0 || isnan(cfactor(unitp))) && (unitp&0xff00)<0xfd00)
    unitp=(unitp&0xffff0000)|0xfe00;
 if (msystem&2)
    if ((unitp&0xff00)==0xfd00)
       table=substtable_usc_big;
    else
       table=substtable_usc_small;
 else
    if ((unitp&0xff00)==0xfd00)
       table=substtable_met_big;
    else
       table=substtable_met_small;
 if (isnan(cfactor(unitp)))
    for (i=0;i<substtable_size;i++)
        if (compatible_units(unitp,table[i]))
           unitp=table[i];
 return unitp;
 }

char *format_meas(double measurement, unsigned int unitp)
{static char output[80],format[80];
 unsigned int base,exp,i;
 double factor,division,m,m2;
 int sign;
 unitp=subst_unit(unitp);
 base=10;
 exp=unitp&31;
 if (exp>15)
    {exp-=16;
     base=2;
     }
 factor=cfactor(unitp);
 division=1;
 for (i=0;i<exp;i++)
     division/=base;
 m=measurement/factor;
 while (fabs(m)>0 && fabs(m)<(exp==2?4:2)*division)
    {division/=base;
     exp++;
     }
 if (base==10)
    {snprintf(format,8,"%%.%df",exp);
     snprintf(output,sizeof(output),format,m);
     }
 else
    {sign=1;
     if (m<0)
        {m=-m;
         sign=-1;
         }
     //printf("%f %f %f %f\n",m,division,m/division,rint(m/division));
     m2=rint(m/division); /* division is a power of 2 so no further worry about roundoff error */
     m=floor(m2*division);
     m2-=m/division;
     while (m2>0 && floor(m2/2)==m2/2)
        {m2/=2;
         division*=2;
         }
     if (m2==0)
        snprintf(output,sizeof(output),"%s%.0f",(sign>0)?"":"-",m);
     else if (m==0)
        snprintf(output,sizeof(output),"%s%.0f/%.0f",(sign>0)?"":"-",m2,1/division);
     else
        snprintf(output,sizeof(output),"%s%.0f+%.0f/%.0f",(sign>0)?"":"-",m,m2,1/division);
     }
 return output;
 }

char *format_meas_unit(double measurement, unsigned int unitp)
{char *output;
 unitp=subst_unit(unitp);
 output=format_meas(measurement,unitp);
 strcat(output," "); /* I can do this because it's static */
 strcat(output,symbol(unitp));
 return output;
 }
 
double parse_meas(char *meas,int unitp,int *found_unit)
/* Given a string representing a measurement, in the unit unitp unless specified otherwise,
   returns its value in the program's internal unit. */
{double intpart,num,denom,factor,result;
 char *meascpy,*intstr,*numstr,*denomstr,*unitstr,*point;
 int i,fmt;
 denomstr=meas+strspn(meas,"0123456789().,+/-$*\\ "); /* temporarily using denomstr for unitstr */
 if (denomstr>meas && denomstr[-1]=='/')
    denomstr--; /* If a slash is just before the symbol, it is part of the symbol. */
 meascpy=(char *)malloc(denomstr-meas+1);
 memset(meascpy,0,denomstr-meas+1);
 strncpy(meascpy,meas,denomstr-meas);
 for (intstr=numstr=meascpy;*numstr;numstr++)
     if (*numstr!=',' && *numstr!='\\' && *numstr!='$' && *numstr!='(' && *numstr!=')' && *numstr!='*')
        *intstr++=*numstr;		/* remove commas and a stray backslash found in a number */
 *intstr=0;
 unitstr=(char *)malloc(strlen(denomstr)+1);
 strcpy(unitstr,denomstr);
 trim(unitstr);
 trim(meascpy);
 if (strchr(meascpy,'/'))
    {denomstr=strchr(meascpy,'/');
     numstr=strchr(meascpy,'+');
     if (!numstr)
        numstr=strchr(meascpy,'-');
     if (numstr)
        {intstr=meascpy;
         *numstr++=0;
         }
     else
        {numstr=meascpy;
         intstr=NULL;
         }
     *denomstr++=0;
     }
 else
    {intstr=meascpy;
     numstr=denomstr=NULL;
     }
 intpart=num=0;
 denom=1;
 if (intstr)
    intpart=atof(intstr);
 if (numstr)
    num=atof(numstr);
 if (denomstr)
    denom=atof(denomstr);
 if (*unitstr)
    {unitp=~unitp;
     for (i=0;i<nsymbols;i++)
         if (!strcmp(symbols[i].symb,unitstr))
            unitp=symbols[i].unitp;
     }
 if (unitp<0)
    {fprintf(stderr,"Unknown unit symbol: %s\n",unitstr);
     factor=0/0;
     unitp=0;
     }
 else
    factor=cfactor(unitp);
 if (denomstr) /* Format is binary fraction. Figure out precision */
    for (fmt=16;fmt<32 && fabs(denom)>(1<<(fmt-16)); fmt++);
 else /* Format is decimal */
    {point=strchr(intstr,'.');
     if (point)
        fmt=strlen(point+1);
     else
        fmt=0;
     }
 unitp=(unitp&0xffffff00)|fmt;
 if (found_unit)
    *found_unit=unitp;
 if (meascpy)
    free(meascpy);
 if (unitstr)
    free (unitstr);
 return (intpart+num/denom)*factor;
 }

double parse_length(const char *meas)
{int found_unit;
 double num;
 char *meascpy;
 meascpy=(char *)malloc(strlen(meas)+1);
 strcpy(meascpy,meas);
 num=parse_meas(meascpy,length_unit,&found_unit);
 free(meascpy);
 return num;
 }
