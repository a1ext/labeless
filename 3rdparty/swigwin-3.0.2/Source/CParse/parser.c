/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 22 "parser.y"

#define yylex yylex

#include "swig.h"
#include "cparse.h"
#include "preprocessor.h"
#include <ctype.h>

/* We do this for portability */
#undef alloca
#define alloca malloc

/* -----------------------------------------------------------------------------
 *                               Externals
 * ----------------------------------------------------------------------------- */

int  yyparse();

/* NEW Variables */

static Node    *top = 0;      /* Top of the generated parse tree */
static int      unnamed = 0;  /* Unnamed datatype counter */
static Hash    *classes = 0;        /* Hash table of classes */
static Hash    *classes_typedefs = 0; /* Hash table of typedef classes: typedef struct X {...} Y; */
static Symtab  *prev_symtab = 0;
static Node    *current_class = 0;
String  *ModuleName = 0;
static Node    *module_node = 0;
static String  *Classprefix = 0;  
static String  *Namespaceprefix = 0;
static int      inclass = 0;
static Node    *currentOuterClass = 0; /* for nested classes */
static char    *last_cpptype = 0;
static int      inherit_list = 0;
static Parm    *template_parameters = 0;
static int      extendmode   = 0;
static int      compact_default_args = 0;
static int      template_reduce = 0;
static int      cparse_externc = 0;
int		ignore_nested_classes = 0;
/* -----------------------------------------------------------------------------
 *                            Assist Functions
 * ----------------------------------------------------------------------------- */


 
/* Called by the parser (yyparse) when an error is found.*/
static void yyerror (const char *e) {
  (void)e;
}

/* Copies a node.  Does not copy tree links or symbol table data (except for
   sym:name) */

static Node *copy_node(Node *n) {
  Node *nn;
  Iterator k;
  nn = NewHash();
  Setfile(nn,Getfile(n));
  Setline(nn,Getline(n));
  for (k = First(n); k.key; k = Next(k)) {
    String *ci;
    String *key = k.key;
    char *ckey = Char(key);
    if ((strcmp(ckey,"nextSibling") == 0) ||
	(strcmp(ckey,"previousSibling") == 0) ||
	(strcmp(ckey,"parentNode") == 0) ||
	(strcmp(ckey,"lastChild") == 0)) {
      continue;
    }
    if (Strncmp(key,"csym:",5) == 0) continue;
    /* We do copy sym:name.  For templates */
    if ((strcmp(ckey,"sym:name") == 0) || 
	(strcmp(ckey,"sym:weak") == 0) ||
	(strcmp(ckey,"sym:typename") == 0)) {
      String *ci = Copy(k.item);
      Setattr(nn,key, ci);
      Delete(ci);
      continue;
    }
    if (strcmp(ckey,"sym:symtab") == 0) {
      Setattr(nn,"sym:needs_symtab", "1");
    }
    /* We don't copy any other symbol table attributes */
    if (strncmp(ckey,"sym:",4) == 0) {
      continue;
    }
    /* If children.  We copy them recursively using this function */
    if (strcmp(ckey,"firstChild") == 0) {
      /* Copy children */
      Node *cn = k.item;
      while (cn) {
	Node *copy = copy_node(cn);
	appendChild(nn,copy);
	Delete(copy);
	cn = nextSibling(cn);
      }
      continue;
    }
    /* We don't copy the symbol table.  But we drop an attribute 
       requires_symtab so that functions know it needs to be built */

    if (strcmp(ckey,"symtab") == 0) {
      /* Node defined a symbol table. */
      Setattr(nn,"requires_symtab","1");
      continue;
    }
    /* Can't copy nodes */
    if (strcmp(ckey,"node") == 0) {
      continue;
    }
    if ((strcmp(ckey,"parms") == 0) || (strcmp(ckey,"pattern") == 0) || (strcmp(ckey,"throws") == 0)
	|| (strcmp(ckey,"kwargs") == 0)) {
      ParmList *pl = CopyParmList(k.item);
      Setattr(nn,key,pl);
      Delete(pl);
      continue;
    }
    if (strcmp(ckey,"nested:outer") == 0) { /* don't copy outer classes links, they will be updated later */
      Setattr(nn, key, k.item);
      continue;
    }
    /* Looks okay.  Just copy the data using Copy */
    ci = Copy(k.item);
    Setattr(nn, key, ci);
    Delete(ci);
  }
  return nn;
}

/* -----------------------------------------------------------------------------
 *                              Variables
 * ----------------------------------------------------------------------------- */

static char  *typemap_lang = 0;    /* Current language setting */

static int cplus_mode  = 0;

/* C++ modes */

#define  CPLUS_PUBLIC    1
#define  CPLUS_PRIVATE   2
#define  CPLUS_PROTECTED 3

/* include types */
static int   import_mode = 0;

void SWIG_typemap_lang(const char *tm_lang) {
  typemap_lang = Swig_copy_string(tm_lang);
}

void SWIG_cparse_set_compact_default_args(int defargs) {
  compact_default_args = defargs;
}

int SWIG_cparse_template_reduce(int treduce) {
  template_reduce = treduce;
  return treduce;  
}

/* -----------------------------------------------------------------------------
 *                           Assist functions
 * ----------------------------------------------------------------------------- */

static int promote_type(int t) {
  if (t <= T_UCHAR || t == T_CHAR) return T_INT;
  return t;
}

/* Perform type-promotion for binary operators */
static int promote(int t1, int t2) {
  t1 = promote_type(t1);
  t2 = promote_type(t2);
  return t1 > t2 ? t1 : t2;
}

static String *yyrename = 0;

/* Forward renaming operator */

static String *resolve_create_node_scope(String *cname);


Hash *Swig_cparse_features(void) {
  static Hash   *features_hash = 0;
  if (!features_hash) features_hash = NewHash();
  return features_hash;
}

/* Fully qualify any template parameters */
static String *feature_identifier_fix(String *s) {
  String *tp = SwigType_istemplate_templateprefix(s);
  if (tp) {
    String *ts, *ta, *tq;
    ts = SwigType_templatesuffix(s);
    ta = SwigType_templateargs(s);
    tq = Swig_symbol_type_qualify(ta,0);
    Append(tp,tq);
    Append(tp,ts);
    Delete(ts);
    Delete(ta);
    Delete(tq);
    return tp;
  } else {
    return NewString(s);
  }
}

static void set_access_mode(Node *n) {
  if (cplus_mode == CPLUS_PUBLIC)
    Setattr(n, "access", "public");
  else if (cplus_mode == CPLUS_PROTECTED)
    Setattr(n, "access", "protected");
  else
    Setattr(n, "access", "private");
}

static void restore_access_mode(Node *n) {
  String *mode = Getattr(n, "access");
  if (Strcmp(mode, "private") == 0)
    cplus_mode = CPLUS_PRIVATE;
  else if (Strcmp(mode, "protected") == 0)
    cplus_mode = CPLUS_PROTECTED;
  else
    cplus_mode = CPLUS_PUBLIC;
}

/* Generate the symbol table name for an object */
/* This is a bit of a mess. Need to clean up */
static String *add_oldname = 0;



static String *make_name(Node *n, String *name,SwigType *decl) {
  int destructor = name && (*(Char(name)) == '~');

  if (yyrename) {
    String *s = NewString(yyrename);
    Delete(yyrename);
    yyrename = 0;
    if (destructor  && (*(Char(s)) != '~')) {
      Insert(s,0,"~");
    }
    return s;
  }

  if (!name) return 0;
  return Swig_name_make(n,Namespaceprefix,name,decl,add_oldname);
}

/* Generate an unnamed identifier */
static String *make_unnamed() {
  unnamed++;
  return NewStringf("$unnamed%d$",unnamed);
}

/* Return if the node is a friend declaration */
static int is_friend(Node *n) {
  return Cmp(Getattr(n,"storage"),"friend") == 0;
}

static int is_operator(String *name) {
  return Strncmp(name,"operator ", 9) == 0;
}


/* Add declaration list to symbol table */
static int  add_only_one = 0;

static void add_symbols(Node *n) {
  String *decl;
  String *wrn = 0;

  if (inclass && n) {
    cparse_normalize_void(n);
  }
  while (n) {
    String *symname = 0;
    /* for friends, we need to pop the scope once */
    String *old_prefix = 0;
    Symtab *old_scope = 0;
    int isfriend = inclass && is_friend(n);
    int iscdecl = Cmp(nodeType(n),"cdecl") == 0;
    int only_csymbol = 0;
    
    if (inclass) {
      String *name = Getattr(n, "name");
      if (isfriend) {
	/* for friends, we need to add the scopename if needed */
	String *prefix = name ? Swig_scopename_prefix(name) : 0;
	old_prefix = Namespaceprefix;
	old_scope = Swig_symbol_popscope();
	Namespaceprefix = Swig_symbol_qualifiedscopename(0);
	if (!prefix) {
	  if (name && !is_operator(name) && Namespaceprefix) {
	    String *nname = NewStringf("%s::%s", Namespaceprefix, name);
	    Setattr(n,"name",nname);
	    Delete(nname);
	  }
	} else {
	  Symtab *st = Swig_symbol_getscope(prefix);
	  String *ns = st ? Getattr(st,"name") : prefix;
	  String *base  = Swig_scopename_last(name);
	  String *nname = NewStringf("%s::%s", ns, base);
	  Setattr(n,"name",nname);
	  Delete(nname);
	  Delete(base);
	  Delete(prefix);
	}
	Namespaceprefix = 0;
      } else {
	/* for member functions, we need to remove the redundant
	   class scope if provided, as in
	   
	   struct Foo {
	   int Foo::method(int a);
	   };
	   
	*/
	String *prefix = name ? Swig_scopename_prefix(name) : 0;
	if (prefix) {
	  if (Classprefix && (Equal(prefix,Classprefix))) {
	    String *base = Swig_scopename_last(name);
	    Setattr(n,"name",base);
	    Delete(base);
	  }
	  Delete(prefix);
	}
      }
    }

    if (!isfriend && (inclass || extendmode)) {
      Setattr(n,"ismember","1");
    }

    if (extendmode) {
      Setattr(n,"isextendmember","1");
    }

    if (!isfriend && inclass) {
      if ((cplus_mode != CPLUS_PUBLIC)) {
	only_csymbol = 1;
	if (cplus_mode == CPLUS_PROTECTED) {
	  Setattr(n,"access", "protected");
	  only_csymbol = !Swig_need_protected(n);
	} else {
	  Setattr(n,"access", "private");
	  /* private are needed only when they are pure virtuals - why? */
	  if ((Cmp(Getattr(n,"storage"),"virtual") == 0) && (Cmp(Getattr(n,"value"),"0") == 0)) {
	    only_csymbol = 0;
	  }
	}
      } else {
	  Setattr(n,"access", "public");
      }
    }
    if (Getattr(n,"sym:name")) {
      n = nextSibling(n);
      continue;
    }
    decl = Getattr(n,"decl");
    if (!SwigType_isfunction(decl)) {
      String *name = Getattr(n,"name");
      String *makename = Getattr(n,"parser:makename");
      if (iscdecl) {	
	String *storage = Getattr(n, "storage");
	if (Cmp(storage,"typedef") == 0) {
	  Setattr(n,"kind","typedef");
	} else {
	  SwigType *type = Getattr(n,"type");
	  String *value = Getattr(n,"value");
	  Setattr(n,"kind","variable");
	  if (value && Len(value)) {
	    Setattr(n,"hasvalue","1");
	  }
	  if (type) {
	    SwigType *ty;
	    SwigType *tmp = 0;
	    if (decl) {
	      ty = tmp = Copy(type);
	      SwigType_push(ty,decl);
	    } else {
	      ty = type;
	    }
	    if (!SwigType_ismutable(ty) || (storage && Strstr(storage, "constexpr"))) {
	      SetFlag(n,"hasconsttype");
	      SetFlag(n,"feature:immutable");
	    }
	    if (tmp) Delete(tmp);
	  }
	  if (!type) {
	    Printf(stderr,"notype name %s\n", name);
	  }
	}
      }
      Swig_features_get(Swig_cparse_features(), Namespaceprefix, name, 0, n);
      if (makename) {
	symname = make_name(n, makename,0);
        Delattr(n,"parser:makename"); /* temporary information, don't leave it hanging around */
      } else {
        makename = name;
	symname = make_name(n, makename,0);
      }
      
      if (!symname) {
	symname = Copy(Getattr(n,"unnamed"));
      }
      if (symname) {
	wrn = Swig_name_warning(n, Namespaceprefix, symname,0);
      }
    } else {
      String *name = Getattr(n,"name");
      SwigType *fdecl = Copy(decl);
      SwigType *fun = SwigType_pop_function(fdecl);
      if (iscdecl) {	
	Setattr(n,"kind","function");
      }
      
      Swig_features_get(Swig_cparse_features(),Namespaceprefix,name,fun,n);

      symname = make_name(n, name,fun);
      wrn = Swig_name_warning(n, Namespaceprefix,symname,fun);
      
      Delete(fdecl);
      Delete(fun);
      
    }
    if (!symname) {
      n = nextSibling(n);
      continue;
    }
    if (cparse_cplusplus) {
      String *value = Getattr(n, "value");
      if (value && Strcmp(value, "delete") == 0) {
	/* C++11 deleted definition / deleted function */
        SetFlag(n,"deleted");
        SetFlag(n,"feature:ignore");
      }
    }
    if (only_csymbol || GetFlag(n,"feature:ignore")) {
      /* Only add to C symbol table and continue */
      Swig_symbol_add(0, n);
    } else if (strncmp(Char(symname),"$ignore",7) == 0) {
      char *c = Char(symname)+7;
      SetFlag(n,"feature:ignore");
      if (strlen(c)) {
	SWIG_WARN_NODE_BEGIN(n);
	Swig_warning(0,Getfile(n), Getline(n), "%s\n",c+1);
	SWIG_WARN_NODE_END(n);
      }
      Swig_symbol_add(0, n);
    } else {
      Node *c;
      if ((wrn) && (Len(wrn))) {
	String *metaname = symname;
	if (!Getmeta(metaname,"already_warned")) {
	  SWIG_WARN_NODE_BEGIN(n);
	  Swig_warning(0,Getfile(n),Getline(n), "%s\n", wrn);
	  SWIG_WARN_NODE_END(n);
	  Setmeta(metaname,"already_warned","1");
	}
      }
      c = Swig_symbol_add(symname,n);

      if (c != n) {
        /* symbol conflict attempting to add in the new symbol */
        if (Getattr(n,"sym:weak")) {
          Setattr(n,"sym:name",symname);
        } else {
          String *e = NewStringEmpty();
          String *en = NewStringEmpty();
          String *ec = NewStringEmpty();
          int redefined = Swig_need_redefined_warn(n,c,inclass);
          if (redefined) {
            Printf(en,"Identifier '%s' redefined (ignored)",symname);
            Printf(ec,"previous definition of '%s'",symname);
          } else {
            Printf(en,"Redundant redeclaration of '%s'",symname);
            Printf(ec,"previous declaration of '%s'",symname);
          }
          if (Cmp(symname,Getattr(n,"name"))) {
            Printf(en," (Renamed from '%s')", SwigType_namestr(Getattr(n,"name")));
          }
          Printf(en,",");
          if (Cmp(symname,Getattr(c,"name"))) {
            Printf(ec," (Renamed from '%s')", SwigType_namestr(Getattr(c,"name")));
          }
          Printf(ec,".");
	  SWIG_WARN_NODE_BEGIN(n);
          if (redefined) {
            Swig_warning(WARN_PARSE_REDEFINED,Getfile(n),Getline(n),"%s\n",en);
            Swig_warning(WARN_PARSE_REDEFINED,Getfile(c),Getline(c),"%s\n",ec);
          } else if (!is_friend(n) && !is_friend(c)) {
            Swig_warning(WARN_PARSE_REDUNDANT,Getfile(n),Getline(n),"%s\n",en);
            Swig_warning(WARN_PARSE_REDUNDANT,Getfile(c),Getline(c),"%s\n",ec);
          }
	  SWIG_WARN_NODE_END(n);
          Printf(e,"%s:%d:%s\n%s:%d:%s\n",Getfile(n),Getline(n),en,
                 Getfile(c),Getline(c),ec);
          Setattr(n,"error",e);
	  Delete(e);
          Delete(en);
          Delete(ec);
        }
      }
    }
    /* restore the class scope if needed */
    if (isfriend) {
      Swig_symbol_setscope(old_scope);
      if (old_prefix) {
	Delete(Namespaceprefix);
	Namespaceprefix = old_prefix;
      }
    }
    Delete(symname);

    if (add_only_one) return;
    n = nextSibling(n);
  }
}


/* add symbols a parse tree node copy */

static void add_symbols_copy(Node *n) {
  String *name;
  int    emode = 0;
  while (n) {
    char *cnodeType = Char(nodeType(n));

    if (strcmp(cnodeType,"access") == 0) {
      String *kind = Getattr(n,"kind");
      if (Strcmp(kind,"public") == 0) {
	cplus_mode = CPLUS_PUBLIC;
      } else if (Strcmp(kind,"private") == 0) {
	cplus_mode = CPLUS_PRIVATE;
      } else if (Strcmp(kind,"protected") == 0) {
	cplus_mode = CPLUS_PROTECTED;
      }
      n = nextSibling(n);
      continue;
    }

    add_oldname = Getattr(n,"sym:name");
    if ((add_oldname) || (Getattr(n,"sym:needs_symtab"))) {
      int old_inclass = -1;
      Node *old_current_class = 0;
      if (add_oldname) {
	DohIncref(add_oldname);
	/*  Disable this, it prevents %rename to work with templates */
	/* If already renamed, we used that name  */
	/*
	if (Strcmp(add_oldname, Getattr(n,"name")) != 0) {
	  Delete(yyrename);
	  yyrename = Copy(add_oldname);
	}
	*/
      }
      Delattr(n,"sym:needs_symtab");
      Delattr(n,"sym:name");

      add_only_one = 1;
      add_symbols(n);

      if (Getattr(n,"partialargs")) {
	Swig_symbol_cadd(Getattr(n,"partialargs"),n);
      }
      add_only_one = 0;
      name = Getattr(n,"name");
      if (Getattr(n,"requires_symtab")) {
	Swig_symbol_newscope();
	Swig_symbol_setscopename(name);
	Delete(Namespaceprefix);
	Namespaceprefix = Swig_symbol_qualifiedscopename(0);
      }
      if (strcmp(cnodeType,"class") == 0) {
	old_inclass = inclass;
	inclass = 1;
	old_current_class = current_class;
	current_class = n;
	if (Strcmp(Getattr(n,"kind"),"class") == 0) {
	  cplus_mode = CPLUS_PRIVATE;
	} else {
	  cplus_mode = CPLUS_PUBLIC;
	}
      }
      if (strcmp(cnodeType,"extend") == 0) {
	emode = cplus_mode;
	cplus_mode = CPLUS_PUBLIC;
      }
      add_symbols_copy(firstChild(n));
      if (strcmp(cnodeType,"extend") == 0) {
	cplus_mode = emode;
      }
      if (Getattr(n,"requires_symtab")) {
	Setattr(n,"symtab", Swig_symbol_popscope());
	Delattr(n,"requires_symtab");
	Delete(Namespaceprefix);
	Namespaceprefix = Swig_symbol_qualifiedscopename(0);
      }
      if (add_oldname) {
	Delete(add_oldname);
	add_oldname = 0;
      }
      if (strcmp(cnodeType,"class") == 0) {
	inclass = old_inclass;
	current_class = old_current_class;
      }
    } else {
      if (strcmp(cnodeType,"extend") == 0) {
	emode = cplus_mode;
	cplus_mode = CPLUS_PUBLIC;
      }
      add_symbols_copy(firstChild(n));
      if (strcmp(cnodeType,"extend") == 0) {
	cplus_mode = emode;
      }
    }
    n = nextSibling(n);
  }
}

/* Check a set of declarations to see if any are pure-abstract */

static List *pure_abstracts(Node *n) {
  List *abstracts = 0;
  while (n) {
    if (Cmp(nodeType(n),"cdecl") == 0) {
      String *decl = Getattr(n,"decl");
      if (SwigType_isfunction(decl)) {
	String *init = Getattr(n,"value");
	if (Cmp(init,"0") == 0) {
	  if (!abstracts) {
	    abstracts = NewList();
	  }
	  Append(abstracts,n);
	  SetFlag(n,"abstract");
	}
      }
    } else if (Cmp(nodeType(n),"destructor") == 0) {
      if (Cmp(Getattr(n,"value"),"0") == 0) {
	if (!abstracts) {
	  abstracts = NewList();
	}
	Append(abstracts,n);
	SetFlag(n,"abstract");
      }
    }
    n = nextSibling(n);
  }
  return abstracts;
}

/* Make a classname */

static String *make_class_name(String *name) {
  String *nname = 0;
  String *prefix;
  if (Namespaceprefix) {
    nname= NewStringf("%s::%s", Namespaceprefix, name);
  } else {
    nname = NewString(name);
  }
  prefix = SwigType_istemplate_templateprefix(nname);
  if (prefix) {
    String *args, *qargs;
    args   = SwigType_templateargs(nname);
    qargs  = Swig_symbol_type_qualify(args,0);
    Append(prefix,qargs);
    Delete(nname);
    Delete(args);
    Delete(qargs);
    nname = prefix;
  }
  return nname;
}

/* Use typedef name as class name */

static void add_typedef_name(Node *n, Node *decl, String *oldName, Symtab *cscope, String *scpname) {
  String *class_rename = 0;
  SwigType *decltype = Getattr(decl, "decl");
  if (!decltype || !Len(decltype)) {
    String *cname;
    String *tdscopename;
    String *class_scope = Swig_symbol_qualifiedscopename(cscope);
    String *name = Getattr(decl, "name");
    cname = Copy(name);
    Setattr(n, "tdname", cname);
    tdscopename = class_scope ? NewStringf("%s::%s", class_scope, name) : Copy(name);
    class_rename = Getattr(n, "class_rename");
    if (class_rename && (Strcmp(class_rename, oldName) == 0))
      Setattr(n, "class_rename", NewString(name));
    if (!classes_typedefs) classes_typedefs = NewHash();
    if (!Equal(scpname, tdscopename) && !Getattr(classes_typedefs, tdscopename)) {
      Setattr(classes_typedefs, tdscopename, n);
    }
    Setattr(n, "decl", decltype);
    Delete(class_scope);
    Delete(cname);
    Delete(tdscopename);
  }
}

/* If the class name is qualified.  We need to create or lookup namespace entries */

static Symtab *set_scope_to_global() {
  Symtab *symtab = Swig_symbol_global_scope();
  Swig_symbol_setscope(symtab);
  return symtab;
}
 
/* Remove the block braces, { and }, if the 'noblock' attribute is set.
 * Node *kw can be either a Hash or Parmlist. */
static String *remove_block(Node *kw, const String *inputcode) {
  String *modified_code = 0;
  while (kw) {
   String *name = Getattr(kw,"name");
   if (name && (Cmp(name,"noblock") == 0)) {
     char *cstr = Char(inputcode);
     size_t len = Len(inputcode);
     if (len && cstr[0] == '{') {
       --len; ++cstr; 
       if (len && cstr[len - 1] == '}') { --len; }
       /* we now remove the extra spaces */
       while (len && isspace((int)cstr[0])) { --len; ++cstr; }
       while (len && isspace((int)cstr[len - 1])) { --len; }
       modified_code = NewStringWithSize(cstr, len);
       break;
     }
   }
   kw = nextSibling(kw);
  }
  return modified_code;
}


static Node *nscope = 0;
static Node *nscope_inner = 0;

/* Remove the scope prefix from cname and return the base name without the prefix.
 * The scopes required for the symbol name are resolved and/or created, if required.
 * For example AA::BB::CC as input returns CC and creates the namespace AA then inner 
 * namespace BB in the current scope. If cname is found to already exist as a weak symbol
 * (forward reference) then the scope might be changed to match, such as when a symbol match 
 * is made via a using reference. */
static String *resolve_create_node_scope(String *cname) {
  Symtab *gscope = 0;
  Node *cname_node = 0;
  int skip_lookup = 0;
  nscope = 0;
  nscope_inner = 0;  

  if (Strncmp(cname,"::",2) == 0)
    skip_lookup = 1;

  cname_node = skip_lookup ? 0 : Swig_symbol_clookup_no_inherit(cname, 0);

  if (cname_node) {
    /* The symbol has been defined already or is in another scope.
       If it is a weak symbol, it needs replacing and if it was brought into the current scope
       via a using declaration, the scope needs adjusting appropriately for the new symbol.
       Similarly for defined templates. */
    Symtab *symtab = Getattr(cname_node, "sym:symtab");
    Node *sym_weak = Getattr(cname_node, "sym:weak");
    if ((symtab && sym_weak) || Equal(nodeType(cname_node), "template")) {
      /* Check if the scope is the current scope */
      String *current_scopename = Swig_symbol_qualifiedscopename(0);
      String *found_scopename = Swig_symbol_qualifiedscopename(symtab);
      int len;
      if (!current_scopename)
	current_scopename = NewString("");
      if (!found_scopename)
	found_scopename = NewString("");
      len = Len(current_scopename);
      if ((len > 0) && (Strncmp(current_scopename, found_scopename, len) == 0)) {
	if (Len(found_scopename) > len + 2) {
	  /* A matching weak symbol was found in non-global scope, some scope adjustment may be required */
	  String *new_cname = NewString(Char(found_scopename) + len + 2); /* skip over "::" prefix */
	  String *base = Swig_scopename_last(cname);
	  Printf(new_cname, "::%s", base);
	  cname = new_cname;
	  Delete(base);
	} else {
	  /* A matching weak symbol was found in the same non-global local scope, no scope adjustment required */
	  assert(len == Len(found_scopename));
	}
      } else {
	String *base = Swig_scopename_last(cname);
	if (Len(found_scopename) > 0) {
	  /* A matching weak symbol was found in a different scope to the local scope - probably via a using declaration */
	  cname = NewStringf("%s::%s", found_scopename, base);
	} else {
	  /* Either:
	      1) A matching weak symbol was found in a different scope to the local scope - this is actually a
	      symbol with the same name in a different scope which we don't want, so no adjustment required.
	      2) A matching weak symbol was found in the global scope - no adjustment required.
	  */
	  cname = Copy(base);
	}
	Delete(base);
      }
      Delete(current_scopename);
      Delete(found_scopename);
    }
  }

  if (Swig_scopename_check(cname)) {
    Node   *ns;
    String *prefix = Swig_scopename_prefix(cname);
    String *base = Swig_scopename_last(cname);
    if (prefix && (Strncmp(prefix,"::",2) == 0)) {
/* I don't think we can use :: global scope to declare classes and hence neither %template. - consider reporting error instead - wsfulton. */
      /* Use the global scope */
      String *nprefix = NewString(Char(prefix)+2);
      Delete(prefix);
      prefix= nprefix;
      gscope = set_scope_to_global();
    }
    if (Len(prefix) == 0) {
      /* Use the global scope, but we need to add a 'global' namespace.  */
      if (!gscope) gscope = set_scope_to_global();
      /* note that this namespace is not the "unnamed" one,
	 and we don't use Setattr(nscope,"name", ""),
	 because the unnamed namespace is private */
      nscope = new_node("namespace");
      Setattr(nscope,"symtab", gscope);;
      nscope_inner = nscope;
      return base;
    }
    /* Try to locate the scope */
    ns = Swig_symbol_clookup(prefix,0);
    if (!ns) {
      Swig_error(cparse_file,cparse_line,"Undefined scope '%s'\n", prefix);
    } else {
      Symtab *nstab = Getattr(ns,"symtab");
      if (!nstab) {
	Swig_error(cparse_file,cparse_line, "'%s' is not defined as a valid scope.\n", prefix);
	ns = 0;
      } else {
	/* Check if the node scope is the current scope */
	String *tname = Swig_symbol_qualifiedscopename(0);
	String *nname = Swig_symbol_qualifiedscopename(nstab);
	if (tname && (Strcmp(tname,nname) == 0)) {
	  ns = 0;
	  cname = base;
	}
	Delete(tname);
	Delete(nname);
      }
      if (ns) {
	/* we will try to create a new node using the namespaces we
	   can find in the scope name */
	List *scopes;
	String *sname;
	Iterator si;
	String *name = NewString(prefix);
	scopes = NewList();
	while (name) {
	  String *base = Swig_scopename_last(name);
	  String *tprefix = Swig_scopename_prefix(name);
	  Insert(scopes,0,base);
	  Delete(base);
	  Delete(name);
	  name = tprefix;
	}
	for (si = First(scopes); si.item; si = Next(si)) {
	  Node *ns1,*ns2;
	  sname = si.item;
	  ns1 = Swig_symbol_clookup(sname,0);
	  assert(ns1);
	  if (Strcmp(nodeType(ns1),"namespace") == 0) {
	    if (Getattr(ns1,"alias")) {
	      ns1 = Getattr(ns1,"namespace");
	    }
	  } else {
	    /* now this last part is a class */
	    si = Next(si);
	    /*  or a nested class tree, which is unrolled here */
	    for (; si.item; si = Next(si)) {
	      if (si.item) {
		Printf(sname,"::%s",si.item);
	      }
	    }
	    /* we get the 'inner' class */
	    nscope_inner = Swig_symbol_clookup(sname,0);
	    /* set the scope to the inner class */
	    Swig_symbol_setscope(Getattr(nscope_inner,"symtab"));
	    /* save the last namespace prefix */
	    Delete(Namespaceprefix);
	    Namespaceprefix = Swig_symbol_qualifiedscopename(0);
	    /* and return the node name, including the inner class prefix */
	    break;
	  }
	  /* here we just populate the namespace tree as usual */
	  ns2 = new_node("namespace");
	  Setattr(ns2,"name",sname);
	  Setattr(ns2,"symtab", Getattr(ns1,"symtab"));
	  add_symbols(ns2);
	  Swig_symbol_setscope(Getattr(ns1,"symtab"));
	  Delete(Namespaceprefix);
	  Namespaceprefix = Swig_symbol_qualifiedscopename(0);
	  if (nscope_inner) {
	    if (Getattr(nscope_inner,"symtab") != Getattr(ns2,"symtab")) {
	      appendChild(nscope_inner,ns2);
	      Delete(ns2);
	    }
	  }
	  nscope_inner = ns2;
	  if (!nscope) nscope = ns2;
	}
	cname = base;
	Delete(scopes);
      }
    }
    Delete(prefix);
  }

  return cname;
}
 
/* look for simple typedef name in typedef list */
static String *try_to_find_a_name_for_unnamed_structure(const char *storage, Node *decls) {
  String *name = 0;
  Node *n = decls;
  if (storage && (strcmp(storage, "typedef") == 0)) {
    for (; n; n = nextSibling(n)) {
      if (!Len(Getattr(n, "decl"))) {
	name = Copy(Getattr(n, "name"));
	break;
      }
    }
  }
  return name;
}

/* traverse copied tree segment, and update outer class links*/
static void update_nested_classes(Node *n)
{
  Node *c = firstChild(n);
  while (c) {
    if (Getattr(c, "nested:outer"))
      Setattr(c, "nested:outer", n);
    update_nested_classes(c);
    c = nextSibling(c);
  }
}

/* -----------------------------------------------------------------------------
 * nested_forward_declaration()
 * 
 * Nested struct handling for C++ code if the nested classes are disabled.
 * Create the nested class/struct/union as a forward declaration.
 * ----------------------------------------------------------------------------- */

static Node *nested_forward_declaration(const char *storage, const char *kind, String *sname, String *name, Node *cpp_opt_declarators) {
  Node *nn = 0;
  int warned = 0;

  if (sname) {
    /* Add forward declaration of the nested type */
    Node *n = new_node("classforward");
    Setattr(n, "kind", kind);
    Setattr(n, "name", sname);
    Setattr(n, "storage", storage);
    Setattr(n, "sym:weak", "1");
    add_symbols(n);
    nn = n;
  }

  /* Add any variable instances. Also add in any further typedefs of the nested type.
     Note that anonymous typedefs (eg typedef struct {...} a, b;) are treated as class forward declarations */
  if (cpp_opt_declarators) {
    int storage_typedef = (storage && (strcmp(storage, "typedef") == 0));
    int variable_of_anonymous_type = !sname && !storage_typedef;
    if (!variable_of_anonymous_type) {
      int anonymous_typedef = !sname && (storage && (strcmp(storage, "typedef") == 0));
      Node *n = cpp_opt_declarators;
      SwigType *type = name;
      while (n) {
	Setattr(n, "type", type);
	Setattr(n, "storage", storage);
	if (anonymous_typedef) {
	  Setattr(n, "nodeType", "classforward");
	  Setattr(n, "sym:weak", "1");
	}
	n = nextSibling(n);
      }
      add_symbols(cpp_opt_declarators);

      if (nn) {
	set_nextSibling(nn, cpp_opt_declarators);
      } else {
	nn = cpp_opt_declarators;
      }
    }
  }

  if (!GetFlag(currentOuterClass, "nested")) {
    if (nn && Equal(nodeType(nn), "classforward")) {
      Node *n = nn;
      SWIG_WARN_NODE_BEGIN(n);
      Swig_warning(WARN_PARSE_NAMED_NESTED_CLASS, cparse_file, cparse_line,"Nested %s not currently supported (%s ignored)\n", kind, sname ? sname : name);
      SWIG_WARN_NODE_END(n);
      warned = 1;
    }

    if (!warned) {
      Swig_warning(WARN_PARSE_UNNAMED_NESTED_CLASS, cparse_file, cparse_line, "Nested %s not currently supported (ignored).\n", kind);
    }
  }

  return nn;
}


Node *Swig_cparse(File *f) {
  scanner_file(f);
  top = 0;
  yyparse();
  return top;
}

static void single_new_feature(const char *featurename, String *val, Hash *featureattribs, char *declaratorid, SwigType *type, ParmList *declaratorparms, String *qualifier) {
  String *fname;
  String *name;
  String *fixname;
  SwigType *t = Copy(type);

  /* Printf(stdout, "single_new_feature: [%s] [%s] [%s] [%s] [%s] [%s]\n", featurename, val, declaratorid, t, ParmList_str_defaultargs(declaratorparms), qualifier); */

  /* Warn about deprecated features */
  if (strcmp(featurename, "nestedworkaround") == 0)
    Swig_warning(WARN_DEPRECATED_NESTED_WORKAROUND, cparse_file, cparse_line, "The 'nestedworkaround' feature is deprecated.\n");

  fname = NewStringf("feature:%s",featurename);
  if (declaratorid) {
    fixname = feature_identifier_fix(declaratorid);
  } else {
    fixname = NewStringEmpty();
  }
  if (Namespaceprefix) {
    name = NewStringf("%s::%s",Namespaceprefix, fixname);
  } else {
    name = fixname;
  }

  if (declaratorparms) Setmeta(val,"parms",declaratorparms);
  if (!Len(t)) t = 0;
  if (t) {
    if (qualifier) SwigType_push(t,qualifier);
    if (SwigType_isfunction(t)) {
      SwigType *decl = SwigType_pop_function(t);
      if (SwigType_ispointer(t)) {
	String *nname = NewStringf("*%s",name);
	Swig_feature_set(Swig_cparse_features(), nname, decl, fname, val, featureattribs);
	Delete(nname);
      } else {
	Swig_feature_set(Swig_cparse_features(), name, decl, fname, val, featureattribs);
      }
      Delete(decl);
    } else if (SwigType_ispointer(t)) {
      String *nname = NewStringf("*%s",name);
      Swig_feature_set(Swig_cparse_features(),nname,0,fname,val, featureattribs);
      Delete(nname);
    }
  } else {
    /* Global feature, that is, feature not associated with any particular symbol */
    Swig_feature_set(Swig_cparse_features(),name,0,fname,val, featureattribs);
  }
  Delete(fname);
  Delete(name);
}

/* Add a new feature to the Hash. Additional features are added if the feature has a parameter list (declaratorparms)
 * and one or more of the parameters have a default argument. An extra feature is added for each defaulted parameter,
 * simulating the equivalent overloaded method. */
static void new_feature(const char *featurename, String *val, Hash *featureattribs, char *declaratorid, SwigType *type, ParmList *declaratorparms, String *qualifier) {

  ParmList *declparms = declaratorparms;

  /* remove the { and } braces if the noblock attribute is set */
  String *newval = remove_block(featureattribs, val);
  val = newval ? newval : val;

  /* Add the feature */
  single_new_feature(featurename, val, featureattribs, declaratorid, type, declaratorparms, qualifier);

  /* Add extra features if there are default parameters in the parameter list */
  if (type) {
    while (declparms) {
      if (ParmList_has_defaultargs(declparms)) {

        /* Create a parameter list for the new feature by copying all
           but the last (defaulted) parameter */
        ParmList* newparms = CopyParmListMax(declparms, ParmList_len(declparms)-1);

        /* Create new declaration - with the last parameter removed */
        SwigType *newtype = Copy(type);
        Delete(SwigType_pop_function(newtype)); /* remove the old parameter list from newtype */
        SwigType_add_function(newtype,newparms);

        single_new_feature(featurename, Copy(val), featureattribs, declaratorid, newtype, newparms, qualifier);
        declparms = newparms;
      } else {
        declparms = 0;
      }
    }
  }
}

/* check if a function declaration is a plain C object */
static int is_cfunction(Node *n) {
  if (!cparse_cplusplus || cparse_externc)
    return 1;
  if (Swig_storage_isexternc(n)) {
    return 1;
  }
  return 0;
}

/* If the Node is a function with parameters, check to see if any of the parameters
 * have default arguments. If so create a new function for each defaulted argument. 
 * The additional functions form a linked list of nodes with the head being the original Node n. */
static void default_arguments(Node *n) {
  Node *function = n;

  if (function) {
    ParmList *varargs = Getattr(function,"feature:varargs");
    if (varargs) {
      /* Handles the %varargs directive by looking for "feature:varargs" and 
       * substituting ... with an alternative set of arguments.  */
      Parm     *p = Getattr(function,"parms");
      Parm     *pp = 0;
      while (p) {
	SwigType *t = Getattr(p,"type");
	if (Strcmp(t,"v(...)") == 0) {
	  if (pp) {
	    ParmList *cv = Copy(varargs);
	    set_nextSibling(pp,cv);
	    Delete(cv);
	  } else {
	    ParmList *cv =  Copy(varargs);
	    Setattr(function,"parms", cv);
	    Delete(cv);
	  }
	  break;
	}
	pp = p;
	p = nextSibling(p);
      }
    }

    /* Do not add in functions if kwargs is being used or if user wants old default argument wrapping
       (one wrapped method per function irrespective of number of default arguments) */
    if (compact_default_args 
	|| is_cfunction(function) 
	|| GetFlag(function,"feature:compactdefaultargs") 
	|| GetFlag(function,"feature:kwargs")) {
      ParmList *p = Getattr(function,"parms");
      if (p) 
        Setattr(p,"compactdefargs", "1"); /* mark parameters for special handling */
      function = 0; /* don't add in extra methods */
    }
  }

  while (function) {
    ParmList *parms = Getattr(function,"parms");
    if (ParmList_has_defaultargs(parms)) {

      /* Create a parameter list for the new function by copying all
         but the last (defaulted) parameter */
      ParmList* newparms = CopyParmListMax(parms,ParmList_len(parms)-1);

      /* Create new function and add to symbol table */
      {
	SwigType *ntype = Copy(nodeType(function));
	char *cntype = Char(ntype);
        Node *new_function = new_node(ntype);
        SwigType *decl = Copy(Getattr(function,"decl"));
        int constqualifier = SwigType_isconst(decl);
	String *ccode = Copy(Getattr(function,"code"));
	String *cstorage = Copy(Getattr(function,"storage"));
	String *cvalue = Copy(Getattr(function,"value"));
	SwigType *ctype = Copy(Getattr(function,"type"));
	String *cthrow = Copy(Getattr(function,"throw"));

        Delete(SwigType_pop_function(decl)); /* remove the old parameter list from decl */
        SwigType_add_function(decl,newparms);
        if (constqualifier)
          SwigType_add_qualifier(decl,"const");

        Setattr(new_function,"name", Getattr(function,"name"));
        Setattr(new_function,"code", ccode);
        Setattr(new_function,"decl", decl);
        Setattr(new_function,"parms", newparms);
        Setattr(new_function,"storage", cstorage);
        Setattr(new_function,"value", cvalue);
        Setattr(new_function,"type", ctype);
        Setattr(new_function,"throw", cthrow);

	Delete(ccode);
	Delete(cstorage);
	Delete(cvalue);
	Delete(ctype);
	Delete(cthrow);
	Delete(decl);

        {
          Node *throws = Getattr(function,"throws");
	  ParmList *pl = CopyParmList(throws);
          if (throws) Setattr(new_function,"throws",pl);
	  Delete(pl);
        }

        /* copy specific attributes for global (or in a namespace) template functions - these are not templated class methods */
        if (strcmp(cntype,"template") == 0) {
          Node *templatetype = Getattr(function,"templatetype");
          Node *symtypename = Getattr(function,"sym:typename");
          Parm *templateparms = Getattr(function,"templateparms");
          if (templatetype) {
	    Node *tmp = Copy(templatetype);
	    Setattr(new_function,"templatetype",tmp);
	    Delete(tmp);
	  }
          if (symtypename) {
	    Node *tmp = Copy(symtypename);
	    Setattr(new_function,"sym:typename",tmp);
	    Delete(tmp);
	  }
          if (templateparms) {
	    Parm *tmp = CopyParmList(templateparms);
	    Setattr(new_function,"templateparms",tmp);
	    Delete(tmp);
	  }
        } else if (strcmp(cntype,"constructor") == 0) {
          /* only copied for constructors as this is not a user defined feature - it is hard coded in the parser */
          if (GetFlag(function,"feature:new")) SetFlag(new_function,"feature:new");
        }

        add_symbols(new_function);
        /* mark added functions as ones with overloaded parameters and point to the parsed method */
        Setattr(new_function,"defaultargs", n);

        /* Point to the new function, extending the linked list */
        set_nextSibling(function, new_function);
	Delete(new_function);
        function = new_function;
	
	Delete(ntype);
      }
    } else {
      function = 0;
    }
  }
}

/* -----------------------------------------------------------------------------
 * mark_nodes_as_extend()
 *
 * Used by the %extend to mark subtypes with "feature:extend".
 * template instances declared within %extend are skipped
 * ----------------------------------------------------------------------------- */

static void mark_nodes_as_extend(Node *n) {
  for (; n; n = nextSibling(n)) {
    if (Getattr(n, "template") && Strcmp(nodeType(n), "class") == 0)
      continue;
    /* Fix me: extend is not a feature. Replace with isextendmember? */
    Setattr(n, "feature:extend", "1");
    mark_nodes_as_extend(firstChild(n));
  }
}



/* Line 268 of yacc.c  */
#line 1348 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ID = 258,
     HBLOCK = 259,
     POUND = 260,
     STRING = 261,
     WSTRING = 262,
     INCLUDE = 263,
     IMPORT = 264,
     INSERT = 265,
     CHARCONST = 266,
     WCHARCONST = 267,
     NUM_INT = 268,
     NUM_FLOAT = 269,
     NUM_UNSIGNED = 270,
     NUM_LONG = 271,
     NUM_ULONG = 272,
     NUM_LONGLONG = 273,
     NUM_ULONGLONG = 274,
     NUM_BOOL = 275,
     TYPEDEF = 276,
     TYPE_INT = 277,
     TYPE_UNSIGNED = 278,
     TYPE_SHORT = 279,
     TYPE_LONG = 280,
     TYPE_FLOAT = 281,
     TYPE_DOUBLE = 282,
     TYPE_CHAR = 283,
     TYPE_WCHAR = 284,
     TYPE_VOID = 285,
     TYPE_SIGNED = 286,
     TYPE_BOOL = 287,
     TYPE_COMPLEX = 288,
     TYPE_TYPEDEF = 289,
     TYPE_RAW = 290,
     TYPE_NON_ISO_INT8 = 291,
     TYPE_NON_ISO_INT16 = 292,
     TYPE_NON_ISO_INT32 = 293,
     TYPE_NON_ISO_INT64 = 294,
     LPAREN = 295,
     RPAREN = 296,
     COMMA = 297,
     SEMI = 298,
     EXTERN = 299,
     INIT = 300,
     LBRACE = 301,
     RBRACE = 302,
     PERIOD = 303,
     CONST_QUAL = 304,
     VOLATILE = 305,
     REGISTER = 306,
     STRUCT = 307,
     UNION = 308,
     EQUAL = 309,
     SIZEOF = 310,
     MODULE = 311,
     LBRACKET = 312,
     RBRACKET = 313,
     BEGINFILE = 314,
     ENDOFFILE = 315,
     ILLEGAL = 316,
     CONSTANT = 317,
     NAME = 318,
     RENAME = 319,
     NAMEWARN = 320,
     EXTEND = 321,
     PRAGMA = 322,
     FEATURE = 323,
     VARARGS = 324,
     ENUM = 325,
     CLASS = 326,
     TYPENAME = 327,
     PRIVATE = 328,
     PUBLIC = 329,
     PROTECTED = 330,
     COLON = 331,
     STATIC = 332,
     VIRTUAL = 333,
     FRIEND = 334,
     THROW = 335,
     CATCH = 336,
     EXPLICIT = 337,
     STATIC_ASSERT = 338,
     CONSTEXPR = 339,
     THREAD_LOCAL = 340,
     DECLTYPE = 341,
     AUTO = 342,
     NOEXCEPT = 343,
     OVERRIDE = 344,
     FINAL = 345,
     USING = 346,
     NAMESPACE = 347,
     NATIVE = 348,
     INLINE = 349,
     TYPEMAP = 350,
     EXCEPT = 351,
     ECHO = 352,
     APPLY = 353,
     CLEAR = 354,
     SWIGTEMPLATE = 355,
     FRAGMENT = 356,
     WARN = 357,
     LESSTHAN = 358,
     GREATERTHAN = 359,
     DELETE_KW = 360,
     DEFAULT = 361,
     LESSTHANOREQUALTO = 362,
     GREATERTHANOREQUALTO = 363,
     EQUALTO = 364,
     NOTEQUALTO = 365,
     ARROW = 366,
     QUESTIONMARK = 367,
     TYPES = 368,
     PARMS = 369,
     NONID = 370,
     DSTAR = 371,
     DCNOT = 372,
     TEMPLATE = 373,
     OPERATOR = 374,
     COPERATOR = 375,
     PARSETYPE = 376,
     PARSEPARM = 377,
     PARSEPARMS = 378,
     CAST = 379,
     LOR = 380,
     LAND = 381,
     OR = 382,
     XOR = 383,
     AND = 384,
     RSHIFT = 385,
     LSHIFT = 386,
     MINUS = 387,
     PLUS = 388,
     MODULO = 389,
     SLASH = 390,
     STAR = 391,
     LNOT = 392,
     NOT = 393,
     UMINUS = 394,
     DCOLON = 395
   };
#endif
/* Tokens.  */
#define ID 258
#define HBLOCK 259
#define POUND 260
#define STRING 261
#define WSTRING 262
#define INCLUDE 263
#define IMPORT 264
#define INSERT 265
#define CHARCONST 266
#define WCHARCONST 267
#define NUM_INT 268
#define NUM_FLOAT 269
#define NUM_UNSIGNED 270
#define NUM_LONG 271
#define NUM_ULONG 272
#define NUM_LONGLONG 273
#define NUM_ULONGLONG 274
#define NUM_BOOL 275
#define TYPEDEF 276
#define TYPE_INT 277
#define TYPE_UNSIGNED 278
#define TYPE_SHORT 279
#define TYPE_LONG 280
#define TYPE_FLOAT 281
#define TYPE_DOUBLE 282
#define TYPE_CHAR 283
#define TYPE_WCHAR 284
#define TYPE_VOID 285
#define TYPE_SIGNED 286
#define TYPE_BOOL 287
#define TYPE_COMPLEX 288
#define TYPE_TYPEDEF 289
#define TYPE_RAW 290
#define TYPE_NON_ISO_INT8 291
#define TYPE_NON_ISO_INT16 292
#define TYPE_NON_ISO_INT32 293
#define TYPE_NON_ISO_INT64 294
#define LPAREN 295
#define RPAREN 296
#define COMMA 297
#define SEMI 298
#define EXTERN 299
#define INIT 300
#define LBRACE 301
#define RBRACE 302
#define PERIOD 303
#define CONST_QUAL 304
#define VOLATILE 305
#define REGISTER 306
#define STRUCT 307
#define UNION 308
#define EQUAL 309
#define SIZEOF 310
#define MODULE 311
#define LBRACKET 312
#define RBRACKET 313
#define BEGINFILE 314
#define ENDOFFILE 315
#define ILLEGAL 316
#define CONSTANT 317
#define NAME 318
#define RENAME 319
#define NAMEWARN 320
#define EXTEND 321
#define PRAGMA 322
#define FEATURE 323
#define VARARGS 324
#define ENUM 325
#define CLASS 326
#define TYPENAME 327
#define PRIVATE 328
#define PUBLIC 329
#define PROTECTED 330
#define COLON 331
#define STATIC 332
#define VIRTUAL 333
#define FRIEND 334
#define THROW 335
#define CATCH 336
#define EXPLICIT 337
#define STATIC_ASSERT 338
#define CONSTEXPR 339
#define THREAD_LOCAL 340
#define DECLTYPE 341
#define AUTO 342
#define NOEXCEPT 343
#define OVERRIDE 344
#define FINAL 345
#define USING 346
#define NAMESPACE 347
#define NATIVE 348
#define INLINE 349
#define TYPEMAP 350
#define EXCEPT 351
#define ECHO 352
#define APPLY 353
#define CLEAR 354
#define SWIGTEMPLATE 355
#define FRAGMENT 356
#define WARN 357
#define LESSTHAN 358
#define GREATERTHAN 359
#define DELETE_KW 360
#define DEFAULT 361
#define LESSTHANOREQUALTO 362
#define GREATERTHANOREQUALTO 363
#define EQUALTO 364
#define NOTEQUALTO 365
#define ARROW 366
#define QUESTIONMARK 367
#define TYPES 368
#define PARMS 369
#define NONID 370
#define DSTAR 371
#define DCNOT 372
#define TEMPLATE 373
#define OPERATOR 374
#define COPERATOR 375
#define PARSETYPE 376
#define PARSEPARM 377
#define PARSEPARMS 378
#define CAST 379
#define LOR 380
#define LAND 381
#define OR 382
#define XOR 383
#define AND 384
#define RSHIFT 385
#define LSHIFT 386
#define MINUS 387
#define PLUS 388
#define MODULO 389
#define SLASH 390
#define STAR 391
#define LNOT 392
#define NOT 393
#define UMINUS 394
#define DCOLON 395




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 1298 "parser.y"

  char  *id;
  List  *bases;
  struct Define {
    String *val;
    String *rawval;
    int     type;
    String *qualifier;
    String *bitfield;
    Parm   *throws;
    String *throwf;
    String *nexcept;
  } dtype;
  struct {
    const char *type;
    String *filename;
    int   line;
  } loc;
  struct {
    char      *id;
    SwigType  *type;
    String    *defarg;
    ParmList  *parms;
    short      have_parms;
    ParmList  *throws;
    String    *throwf;
    String    *nexcept;
  } decl;
  Parm         *tparms;
  struct {
    String     *method;
    Hash       *kwargs;
  } tmap;
  struct {
    String     *type;
    String     *us;
  } ptype;
  SwigType     *type;
  String       *str;
  Parm         *p;
  ParmList     *pl;
  int           intvalue;
  Node         *node;



/* Line 293 of yacc.c  */
#line 1711 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 1723 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  61
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   5166

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  141
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  167
/* YYNRULES -- Number of rules.  */
#define YYNRULES  568
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1113

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   395

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    12,    16,    19,    25,    29,
      32,    34,    36,    38,    40,    42,    44,    46,    49,    51,
      53,    55,    57,    59,    61,    63,    65,    67,    69,    71,
      73,    75,    77,    79,    81,    83,    85,    87,    89,    91,
      92,   100,   106,   110,   116,   122,   126,   129,   132,   138,
     141,   147,   150,   155,   157,   159,   167,   175,   181,   182,
     190,   192,   194,   197,   200,   202,   208,   214,   220,   224,
     229,   233,   241,   250,   256,   260,   262,   264,   268,   270,
     275,   283,   290,   292,   294,   302,   312,   321,   332,   338,
     346,   353,   362,   364,   366,   372,   377,   383,   391,   393,
     397,   404,   411,   420,   422,   425,   429,   431,   434,   438,
     445,   451,   461,   464,   466,   468,   470,   471,   478,   480,
     484,   493,   499,   507,   509,   514,   516,   518,   521,   524,
     528,   530,   532,   534,   537,   539,   541,   543,   555,   569,
     577,   579,   581,   583,   584,   588,   590,   593,   596,   599,
     601,   607,   616,   627,   634,   636,   638,   640,   642,   644,
     646,   647,   657,   658,   667,   669,   673,   678,   679,   686,
     690,   695,   697,   699,   701,   703,   705,   707,   709,   711,
     714,   716,   718,   720,   724,   726,   730,   735,   736,   743,
     744,   750,   756,   759,   760,   761,   769,   771,   773,   774,
     778,   780,   782,   784,   786,   788,   790,   792,   794,   796,
     800,   802,   804,   806,   808,   810,   812,   814,   816,   818,
     825,   832,   840,   849,   858,   867,   877,   885,   891,   894,
     897,   900,   903,   905,   907,   909,   911,   913,   915,   917,
     919,   921,   923,   925,   928,   933,   936,   939,   944,   947,
     953,   955,   957,   959,   962,   964,   966,   968,   971,   975,
     977,   979,   981,   983,   985,   987,   990,   992,   995,   998,
    1001,  1004,  1006,  1008,  1011,  1013,  1017,  1019,  1022,  1030,
    1034,  1036,  1039,  1041,  1045,  1047,  1049,  1051,  1054,  1060,
    1063,  1066,  1068,  1071,  1074,  1076,  1078,  1080,  1082,  1085,
    1089,  1093,  1095,  1098,  1101,  1105,  1110,  1116,  1121,  1127,
    1134,  1141,  1146,  1152,  1158,  1165,  1173,  1182,  1191,  1199,
    1207,  1209,  1212,  1216,  1221,  1227,  1231,  1236,  1241,  1243,
    1246,  1251,  1256,  1261,  1267,  1271,  1276,  1281,  1287,  1289,
    1292,  1295,  1298,  1302,  1306,  1308,  1311,  1314,  1316,  1318,
    1321,  1325,  1330,  1334,  1339,  1342,  1346,  1350,  1355,  1359,
    1363,  1366,  1369,  1371,  1373,  1376,  1378,  1380,  1382,  1384,
    1387,  1389,  1392,  1396,  1398,  1400,  1402,  1405,  1408,  1410,
    1412,  1415,  1417,  1422,  1424,  1426,  1429,  1431,  1433,  1435,
    1437,  1439,  1441,  1443,  1445,  1447,  1449,  1451,  1453,  1455,
    1457,  1458,  1461,  1463,  1465,  1467,  1469,  1471,  1473,  1475,
    1477,  1479,  1485,  1489,  1493,  1495,  1497,  1501,  1503,  1505,
    1507,  1509,  1511,  1517,  1526,  1528,  1530,  1532,  1534,  1538,
    1543,  1549,  1555,  1561,  1568,  1575,  1578,  1581,  1584,  1586,
    1588,  1590,  1592,  1594,  1596,  1598,  1600,  1604,  1608,  1612,
    1616,  1620,  1624,  1628,  1632,  1636,  1640,  1644,  1648,  1652,
    1656,  1660,  1664,  1670,  1673,  1676,  1679,  1682,  1685,  1689,
    1691,  1693,  1695,  1696,  1700,  1702,  1704,  1708,  1709,  1714,
    1715,  1722,  1724,  1726,  1728,  1730,  1732,  1737,  1742,  1744,
    1746,  1748,  1750,  1752,  1754,  1756,  1759,  1762,  1767,  1769,
    1771,  1774,  1779,  1781,  1783,  1786,  1788,  1792,  1796,  1801,
    1806,  1810,  1815,  1818,  1820,  1822,  1826,  1831,  1838,  1841,
    1844,  1848,  1850,  1852,  1854,  1856,  1858,  1860,  1862,  1864,
    1866,  1869,  1874,  1876,  1880,  1882,  1886,  1890,  1893,  1896,
    1899,  1902,  1905,  1910,  1912,  1916,  1918,  1922,  1926,  1929,
    1932,  1935,  1938,  1940,  1943,  1945,  1947,  1949,  1951,  1955,
    1957,  1961,  1967,  1969,  1973,  1977,  1983,  1985,  1987
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     142,     0,    -1,   143,    -1,   121,   235,    43,    -1,   121,
       1,    -1,   122,   235,    43,    -1,   122,     1,    -1,   123,
      40,   232,    41,    43,    -1,   123,     1,    43,    -1,   143,
     144,    -1,   307,    -1,   145,    -1,   182,    -1,   198,    -1,
      43,    -1,     1,    -1,   197,    -1,     1,   120,    -1,   146,
      -1,   148,    -1,   149,    -1,   150,    -1,   151,    -1,   152,
      -1,   155,    -1,   156,    -1,   159,    -1,   160,    -1,   161,
      -1,   162,    -1,   163,    -1,   164,    -1,   167,    -1,   169,
      -1,   172,    -1,   174,    -1,   179,    -1,   180,    -1,   181,
      -1,    -1,    66,   304,   296,    46,   147,   215,    47,    -1,
      98,   178,    46,   176,    47,    -1,    99,   176,    43,    -1,
      62,   293,    54,   258,    43,    -1,    62,   251,   243,   240,
      43,    -1,    62,     1,    43,    -1,    97,     4,    -1,    97,
     301,    -1,    96,    40,   293,    41,    46,    -1,    96,    46,
      -1,    96,    40,   293,    41,    43,    -1,    96,    43,    -1,
     301,    46,   235,    47,    -1,   301,    -1,   153,    -1,   101,
      40,   154,    42,   305,    41,     4,    -1,   101,    40,   154,
      42,   305,    41,    46,    -1,   101,    40,   154,    41,    43,
      -1,    -1,   158,   304,   301,    59,   157,   143,    60,    -1,
       8,    -1,     9,    -1,    94,     4,    -1,    94,    46,    -1,
       4,    -1,    10,    40,   294,    41,   301,    -1,    10,    40,
     294,    41,     4,    -1,    10,    40,   294,    41,    46,    -1,
      56,   304,   294,    -1,    63,    40,   294,    41,    -1,    63,
      40,    41,    -1,    93,    40,   293,    41,   231,   293,    43,
      -1,    93,    40,   293,    41,   231,   251,   243,    43,    -1,
      67,   166,   293,    54,   165,    -1,    67,   166,   293,    -1,
     301,    -1,     4,    -1,    40,   293,    41,    -1,   307,    -1,
     168,   243,   294,    43,    -1,   168,    40,   305,    41,   243,
     287,    43,    -1,   168,    40,   305,    41,   301,    43,    -1,
      64,    -1,    65,    -1,    68,    40,   294,    41,   243,   287,
     170,    -1,    68,    40,   294,    42,   306,    41,   243,   287,
      43,    -1,    68,    40,   294,   171,    41,   243,   287,   170,
      -1,    68,    40,   294,    42,   306,   171,    41,   243,   287,
      43,    -1,    68,    40,   294,    41,   170,    -1,    68,    40,
     294,    42,   306,    41,    43,    -1,    68,    40,   294,   171,
      41,   170,    -1,    68,    40,   294,    42,   306,   171,    41,
      43,    -1,   303,    -1,    43,    -1,   114,    40,   232,    41,
      43,    -1,    42,   294,    54,   306,    -1,    42,   294,    54,
     306,   171,    -1,    69,    40,   173,    41,   243,   287,    43,
      -1,   232,    -1,    13,    42,   235,    -1,    95,    40,   175,
      41,   176,   303,    -1,    95,    40,   175,    41,   176,    43,
      -1,    95,    40,   175,    41,   176,    54,   178,    43,    -1,
     305,    -1,   178,   177,    -1,    42,   178,   177,    -1,   307,
      -1,   251,   242,    -1,    40,   232,    41,    -1,    40,   232,
      41,    40,   232,    41,    -1,   113,    40,   232,    41,   170,
      -1,   100,    40,   295,    41,   299,   103,   236,   104,    43,
      -1,   102,   301,    -1,   184,    -1,   196,    -1,   195,    -1,
      -1,    44,   301,    46,   183,   143,    47,    -1,   188,    -1,
      91,   296,    54,    -1,   118,   103,   207,   104,    91,   296,
      54,   293,    -1,   231,   251,   243,   186,   185,    -1,   231,
      87,   243,   111,   187,   186,   185,    -1,    43,    -1,    42,
     243,   186,   185,    -1,    46,    -1,   240,    -1,   249,   240,
      -1,   286,   240,    -1,   249,   286,   240,    -1,   255,    -1,
      32,    -1,    30,    -1,    34,   292,    -1,    35,    -1,   296,
      -1,   254,    -1,   231,    87,   296,    54,   189,    40,   232,
      41,   287,   190,   191,    -1,   231,    87,   296,    54,   189,
      40,   232,    41,   287,   111,   251,   190,   191,    -1,   231,
      87,   296,    54,   189,   190,   191,    -1,    57,    -1,    46,
      -1,    43,    -1,    -1,    40,   192,    43,    -1,    70,    -1,
      70,    71,    -1,    70,    52,    -1,    76,   253,    -1,   307,
      -1,   231,   193,   263,   194,    43,    -1,   231,   193,   263,
     194,    46,   265,    47,    43,    -1,   231,   193,   263,   194,
      46,   265,    47,   243,   186,   185,    -1,   231,   251,    40,
     232,    41,   288,    -1,   199,    -1,   203,    -1,   204,    -1,
     211,    -1,   212,    -1,   223,    -1,    -1,   231,   283,   296,
     274,    46,   200,   215,    47,   202,    -1,    -1,   231,   283,
     274,    46,   201,   215,    47,   202,    -1,    43,    -1,   243,
     186,   185,    -1,   231,   283,   296,    43,    -1,    -1,   118,
     103,   207,   104,   205,   206,    -1,   118,   283,   296,    -1,
      44,   118,   283,   296,    -1,   184,    -1,   199,    -1,   220,
      -1,   224,    -1,   204,    -1,   203,    -1,   222,    -1,   208,
      -1,   209,   210,    -1,   307,    -1,   282,    -1,   235,    -1,
      42,   209,   210,    -1,   307,    -1,    91,   296,    43,    -1,
      91,    92,   296,    43,    -1,    -1,    92,   296,    46,   213,
     143,    47,    -1,    -1,    92,    46,   214,   143,    47,    -1,
      92,   293,    54,   296,    43,    -1,   219,   215,    -1,    -1,
      -1,    66,    46,   216,   215,    47,   217,   215,    -1,   156,
      -1,   307,    -1,    -1,     1,   218,   215,    -1,   182,    -1,
     220,    -1,   221,    -1,   224,    -1,   225,    -1,   226,    -1,
     222,    -1,   203,    -1,   199,    -1,   231,   296,    43,    -1,
     211,    -1,   204,    -1,   223,    -1,   180,    -1,   181,    -1,
     229,    -1,   155,    -1,   179,    -1,    43,    -1,   231,   251,
      40,   232,    41,   288,    -1,   138,   298,    40,   232,    41,
     227,    -1,    78,   138,   298,    40,   232,    41,   228,    -1,
     231,   120,   251,   248,    40,   232,    41,   228,    -1,   231,
     120,   251,   129,    40,   232,    41,   228,    -1,   231,   120,
     251,   126,    40,   232,    41,   228,    -1,   231,   120,   251,
     248,   129,    40,   232,    41,   228,    -1,   231,   120,   251,
      40,   232,    41,   228,    -1,    81,    40,   232,    41,    46,
      -1,    83,    40,    -1,    74,    76,    -1,    73,    76,    -1,
      75,    76,    -1,   164,    -1,   150,    -1,   162,    -1,   167,
      -1,   169,    -1,   172,    -1,   160,    -1,   174,    -1,   148,
      -1,   149,    -1,   151,    -1,   287,    43,    -1,   287,    54,
     260,    43,    -1,   287,    46,    -1,   287,    43,    -1,   287,
      54,   258,    43,    -1,   287,    46,    -1,   231,   230,    76,
     268,    43,    -1,   255,    -1,    32,    -1,    30,    -1,    34,
     292,    -1,    35,    -1,   296,    -1,    44,    -1,    44,   301,
      -1,    44,   301,    85,    -1,    77,    -1,    21,    -1,    78,
      -1,    79,    -1,    82,    -1,    84,    -1,    77,    84,    -1,
      85,    -1,    85,    77,    -1,    77,    85,    -1,    44,    85,
      -1,    85,    44,    -1,   307,    -1,   233,    -1,   235,   234,
      -1,   307,    -1,    42,   235,   234,    -1,   307,    -1,   252,
     241,    -1,   118,   103,   283,   104,   283,   296,   240,    -1,
      48,    48,    48,    -1,   237,    -1,   239,   238,    -1,   307,
      -1,    42,   239,   238,    -1,   307,    -1,   235,    -1,   269,
      -1,    54,   258,    -1,    54,   258,    57,   268,    58,    -1,
      54,    46,    -1,    76,   268,    -1,   307,    -1,   243,   240,
      -1,   246,   240,    -1,   240,    -1,   243,    -1,   246,    -1,
     307,    -1,   248,   244,    -1,   248,   129,   244,    -1,   248,
     126,   244,    -1,   245,    -1,   129,   244,    -1,   126,   244,
      -1,   296,   116,   244,    -1,   248,   296,   116,   244,    -1,
     248,   296,   116,   129,   244,    -1,   296,   116,   129,   244,
      -1,   248,    48,    48,    48,   244,    -1,   248,   129,    48,
      48,    48,   244,    -1,   248,   126,    48,    48,    48,   244,
      -1,    48,    48,    48,   245,    -1,   129,    48,    48,    48,
     244,    -1,   126,    48,    48,    48,   244,    -1,   296,   116,
      48,    48,    48,   244,    -1,   248,   296,   116,    48,    48,
      48,   244,    -1,   248,   296,   116,   129,    48,    48,    48,
     244,    -1,   248,   296,   116,   126,    48,    48,    48,   244,
      -1,   296,   116,   129,    48,    48,    48,   244,    -1,   296,
     116,   126,    48,    48,    48,   244,    -1,   296,    -1,   138,
     296,    -1,    40,   296,    41,    -1,    40,   248,   244,    41,
      -1,    40,   296,   116,   244,    41,    -1,   244,    57,    58,
      -1,   244,    57,   268,    58,    -1,   244,    40,   232,    41,
      -1,   296,    -1,   138,   296,    -1,    40,   248,   245,    41,
      -1,    40,   129,   245,    41,    -1,    40,   126,   245,    41,
      -1,    40,   296,   116,   245,    41,    -1,   245,    57,    58,
      -1,   245,    57,   268,    58,    -1,   245,    40,   232,    41,
      -1,   119,     3,    40,   232,    41,    -1,   248,    -1,   248,
     247,    -1,   248,   129,    -1,   248,   126,    -1,   248,   129,
     247,    -1,   248,   126,   247,    -1,   247,    -1,   129,   247,
      -1,   126,   247,    -1,   129,    -1,   126,    -1,   296,   116,
      -1,   248,   296,   116,    -1,   248,   296,   116,   247,    -1,
     247,    57,    58,    -1,   247,    57,   268,    58,    -1,    57,
      58,    -1,    57,   268,    58,    -1,    40,   246,    41,    -1,
     247,    40,   232,    41,    -1,    40,   232,    41,    -1,   136,
     249,   248,    -1,   136,   248,    -1,   136,   249,    -1,   136,
      -1,   250,    -1,   250,   249,    -1,    49,    -1,    50,    -1,
      51,    -1,   252,    -1,   249,   253,    -1,   253,    -1,   253,
     249,    -1,   249,   253,   249,    -1,   255,    -1,    32,    -1,
      30,    -1,    34,   292,    -1,   193,   296,    -1,    35,    -1,
     296,    -1,   283,   296,    -1,   254,    -1,    86,    40,   296,
      41,    -1,   256,    -1,   257,    -1,   257,   256,    -1,    22,
      -1,    24,    -1,    25,    -1,    28,    -1,    29,    -1,    26,
      -1,    27,    -1,    31,    -1,    23,    -1,    33,    -1,    36,
      -1,    37,    -1,    38,    -1,    39,    -1,    -1,   259,   268,
      -1,   260,    -1,   261,    -1,   262,    -1,   105,    -1,   106,
      -1,   293,    -1,   307,    -1,   150,    -1,   307,    -1,   265,
      42,   264,   266,   264,    -1,   265,    42,   264,    -1,   264,
     266,   264,    -1,   264,    -1,   293,    -1,   293,    54,   267,
      -1,   268,    -1,   269,    -1,   251,    -1,   270,    -1,   301,
      -1,    55,    40,   251,   241,    41,    -1,    55,    48,    48,
      48,    40,   251,   241,    41,    -1,   271,    -1,   302,    -1,
      11,    -1,    12,    -1,    40,   268,    41,    -1,    40,   268,
      41,   268,    -1,    40,   268,   248,    41,   268,    -1,    40,
     268,   129,    41,   268,    -1,    40,   268,   126,    41,   268,
      -1,    40,   268,   248,   129,    41,   268,    -1,    40,   268,
     248,   126,    41,   268,    -1,   129,   268,    -1,   126,   268,
      -1,   136,   268,    -1,    13,    -1,    14,    -1,    15,    -1,
      16,    -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,
     268,   133,   268,    -1,   268,   132,   268,    -1,   268,   136,
     268,    -1,   268,   135,   268,    -1,   268,   134,   268,    -1,
     268,   129,   268,    -1,   268,   127,   268,    -1,   268,   128,
     268,    -1,   268,   131,   268,    -1,   268,   130,   268,    -1,
     268,   126,   268,    -1,   268,   125,   268,    -1,   268,   109,
     268,    -1,   268,   110,   268,    -1,   268,   108,   268,    -1,
     268,   107,   268,    -1,   268,   112,   268,    76,   268,    -1,
     132,   268,    -1,   133,   268,    -1,   138,   268,    -1,   137,
     268,    -1,   251,    40,    -1,    48,    48,    48,    -1,   272,
      -1,   307,    -1,   275,    -1,    -1,    76,   276,   277,    -1,
     307,    -1,   278,    -1,   277,    42,   278,    -1,    -1,   284,
     279,   296,   273,    -1,    -1,   284,   281,   280,   284,   296,
     273,    -1,    74,    -1,    73,    -1,    75,    -1,    71,    -1,
      72,    -1,    71,    48,    48,    48,    -1,    72,    48,    48,
      48,    -1,   282,    -1,    52,    -1,    53,    -1,    78,    -1,
     307,    -1,    89,    -1,    90,    -1,    90,    89,    -1,    89,
      90,    -1,    80,    40,   232,    41,    -1,    88,    -1,   285,
      -1,    88,   285,    -1,    88,    40,   268,    41,    -1,   249,
      -1,   286,    -1,   249,   286,    -1,   307,    -1,   287,   289,
      43,    -1,   287,   289,    46,    -1,    40,   232,    41,    43,
      -1,    40,   232,    41,    46,    -1,    54,   258,    43,    -1,
     286,    54,   260,    43,    -1,    76,   290,    -1,   307,    -1,
     291,    -1,   290,    42,   291,    -1,   291,    48,    48,    48,
      -1,   290,    42,   291,    48,    48,    48,    -1,   296,    40,
      -1,   296,    46,    -1,   103,   236,   104,    -1,   307,    -1,
       3,    -1,    89,    -1,    90,    -1,   293,    -1,   260,    -1,
     301,    -1,   294,    -1,   307,    -1,   298,   297,    -1,   115,
     140,   298,   297,    -1,   298,    -1,   115,   140,   298,    -1,
     119,    -1,   115,   140,   119,    -1,   140,   298,   297,    -1,
     140,   298,    -1,   140,   119,    -1,   117,   298,    -1,   293,
     292,    -1,   293,   300,    -1,   115,   140,   293,   300,    -1,
     293,    -1,   115,   140,   293,    -1,   119,    -1,   115,   140,
     119,    -1,   140,   293,   300,    -1,   140,   293,    -1,   140,
     119,    -1,   117,   293,    -1,   301,     6,    -1,     6,    -1,
     302,     7,    -1,     7,    -1,   301,    -1,    46,    -1,     4,
      -1,    40,   305,    41,    -1,   307,    -1,   294,    54,   306,
      -1,   294,    54,   306,    42,   305,    -1,   294,    -1,   294,
      42,   305,    -1,   294,    54,   153,    -1,   294,    54,   153,
      42,   305,    -1,   301,    -1,   270,    -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,  1463,  1463,  1475,  1479,  1482,  1485,  1488,  1491,  1496,
    1501,  1506,  1507,  1508,  1509,  1510,  1516,  1532,  1542,  1543,
    1544,  1545,  1546,  1547,  1548,  1549,  1550,  1551,  1552,  1553,
    1554,  1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,  1569,
    1569,  1651,  1661,  1672,  1693,  1715,  1726,  1735,  1754,  1760,
    1766,  1771,  1778,  1785,  1789,  1802,  1811,  1826,  1839,  1839,
    1894,  1895,  1902,  1921,  1952,  1956,  1966,  1971,  1989,  2032,
    2038,  2051,  2057,  2083,  2089,  2096,  2097,  2100,  2101,  2108,
    2154,  2200,  2211,  2214,  2241,  2247,  2253,  2259,  2267,  2273,
    2279,  2285,  2293,  2294,  2295,  2298,  2303,  2313,  2349,  2350,
    2385,  2402,  2410,  2423,  2448,  2454,  2458,  2461,  2472,  2477,
    2490,  2502,  2788,  2798,  2805,  2806,  2810,  2810,  2835,  2841,
    2852,  2868,  2928,  2986,  2990,  3013,  3019,  3026,  3033,  3040,
    3049,  3050,  3051,  3052,  3053,  3054,  3055,  3066,  3071,  3076,
    3083,  3089,  3094,  3097,  3097,  3110,  3113,  3116,  3125,  3128,
    3135,  3157,  3186,  3284,  3336,  3337,  3338,  3339,  3340,  3341,
    3346,  3346,  3585,  3585,  3730,  3731,  3743,  3761,  3761,  4018,
    4024,  4030,  4033,  4036,  4039,  4042,  4045,  4048,  4053,  4089,
    4093,  4096,  4099,  4104,  4108,  4113,  4123,  4154,  4154,  4183,
    4183,  4205,  4232,  4249,  4254,  4249,  4262,  4263,  4264,  4264,
    4280,  4281,  4298,  4299,  4300,  4301,  4302,  4303,  4304,  4305,
    4306,  4307,  4308,  4309,  4310,  4311,  4312,  4313,  4314,  4323,
    4351,  4378,  4409,  4424,  4441,  4459,  4478,  4497,  4504,  4511,
    4518,  4526,  4534,  4537,  4541,  4544,  4545,  4546,  4547,  4548,
    4549,  4550,  4551,  4554,  4561,  4568,  4577,  4586,  4595,  4607,
    4610,  4613,  4614,  4615,  4616,  4618,  4627,  4628,  4638,  4648,
    4649,  4650,  4651,  4652,  4653,  4654,  4655,  4656,  4657,  4658,
    4659,  4660,  4667,  4678,  4682,  4685,  4689,  4693,  4703,  4711,
    4719,  4732,  4736,  4739,  4743,  4747,  4775,  4783,  4795,  4810,
    4820,  4829,  4840,  4844,  4848,  4855,  4872,  4889,  4897,  4905,
    4914,  4923,  4927,  4936,  4947,  4958,  4970,  4980,  4994,  5002,
    5011,  5020,  5024,  5033,  5044,  5055,  5067,  5077,  5087,  5098,
    5111,  5118,  5126,  5142,  5150,  5161,  5172,  5183,  5202,  5210,
    5227,  5235,  5242,  5249,  5260,  5271,  5282,  5302,  5323,  5329,
    5335,  5342,  5349,  5358,  5367,  5370,  5379,  5388,  5395,  5402,
    5409,  5419,  5430,  5441,  5452,  5459,  5466,  5469,  5486,  5496,
    5503,  5509,  5514,  5520,  5524,  5530,  5531,  5532,  5538,  5544,
    5548,  5549,  5553,  5560,  5563,  5564,  5565,  5566,  5567,  5569,
    5572,  5575,  5580,  5591,  5616,  5619,  5673,  5677,  5681,  5685,
    5689,  5693,  5697,  5701,  5705,  5709,  5713,  5717,  5721,  5725,
    5731,  5731,  5745,  5761,  5764,  5770,  5783,  5797,  5798,  5801,
    5802,  5806,  5812,  5815,  5819,  5824,  5832,  5844,  5859,  5860,
    5879,  5880,  5884,  5889,  5894,  5895,  5900,  5913,  5928,  5935,
    5952,  5959,  5966,  5973,  5981,  5989,  5993,  5997,  6003,  6004,
    6005,  6006,  6007,  6008,  6009,  6010,  6013,  6017,  6021,  6025,
    6029,  6033,  6037,  6041,  6045,  6049,  6053,  6057,  6061,  6065,
    6079,  6083,  6087,  6093,  6097,  6101,  6105,  6109,  6125,  6130,
    6133,  6138,  6143,  6143,  6144,  6147,  6164,  6173,  6173,  6191,
    6191,  6209,  6210,  6211,  6215,  6219,  6223,  6227,  6233,  6236,
    6240,  6246,  6247,  6250,  6253,  6256,  6259,  6264,  6269,  6274,
    6279,  6284,  6291,  6297,  6301,  6305,  6313,  6321,  6329,  6338,
    6347,  6354,  6363,  6364,  6367,  6368,  6369,  6370,  6373,  6385,
    6391,  6397,  6401,  6402,  6403,  6406,  6407,  6408,  6411,  6412,
    6415,  6420,  6424,  6427,  6430,  6433,  6438,  6442,  6445,  6452,
    6458,  6467,  6472,  6476,  6479,  6482,  6485,  6490,  6494,  6497,
    6500,  6506,  6511,  6514,  6526,  6529,  6532,  6536,  6541,  6554,
    6558,  6563,  6569,  6573,  6578,  6582,  6589,  6592,  6597
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ID", "HBLOCK", "POUND", "STRING",
  "WSTRING", "INCLUDE", "IMPORT", "INSERT", "CHARCONST", "WCHARCONST",
  "NUM_INT", "NUM_FLOAT", "NUM_UNSIGNED", "NUM_LONG", "NUM_ULONG",
  "NUM_LONGLONG", "NUM_ULONGLONG", "NUM_BOOL", "TYPEDEF", "TYPE_INT",
  "TYPE_UNSIGNED", "TYPE_SHORT", "TYPE_LONG", "TYPE_FLOAT", "TYPE_DOUBLE",
  "TYPE_CHAR", "TYPE_WCHAR", "TYPE_VOID", "TYPE_SIGNED", "TYPE_BOOL",
  "TYPE_COMPLEX", "TYPE_TYPEDEF", "TYPE_RAW", "TYPE_NON_ISO_INT8",
  "TYPE_NON_ISO_INT16", "TYPE_NON_ISO_INT32", "TYPE_NON_ISO_INT64",
  "LPAREN", "RPAREN", "COMMA", "SEMI", "EXTERN", "INIT", "LBRACE",
  "RBRACE", "PERIOD", "CONST_QUAL", "VOLATILE", "REGISTER", "STRUCT",
  "UNION", "EQUAL", "SIZEOF", "MODULE", "LBRACKET", "RBRACKET",
  "BEGINFILE", "ENDOFFILE", "ILLEGAL", "CONSTANT", "NAME", "RENAME",
  "NAMEWARN", "EXTEND", "PRAGMA", "FEATURE", "VARARGS", "ENUM", "CLASS",
  "TYPENAME", "PRIVATE", "PUBLIC", "PROTECTED", "COLON", "STATIC",
  "VIRTUAL", "FRIEND", "THROW", "CATCH", "EXPLICIT", "STATIC_ASSERT",
  "CONSTEXPR", "THREAD_LOCAL", "DECLTYPE", "AUTO", "NOEXCEPT", "OVERRIDE",
  "FINAL", "USING", "NAMESPACE", "NATIVE", "INLINE", "TYPEMAP", "EXCEPT",
  "ECHO", "APPLY", "CLEAR", "SWIGTEMPLATE", "FRAGMENT", "WARN", "LESSTHAN",
  "GREATERTHAN", "DELETE_KW", "DEFAULT", "LESSTHANOREQUALTO",
  "GREATERTHANOREQUALTO", "EQUALTO", "NOTEQUALTO", "ARROW", "QUESTIONMARK",
  "TYPES", "PARMS", "NONID", "DSTAR", "DCNOT", "TEMPLATE", "OPERATOR",
  "COPERATOR", "PARSETYPE", "PARSEPARM", "PARSEPARMS", "CAST", "LOR",
  "LAND", "OR", "XOR", "AND", "RSHIFT", "LSHIFT", "MINUS", "PLUS",
  "MODULO", "SLASH", "STAR", "LNOT", "NOT", "UMINUS", "DCOLON", "$accept",
  "program", "interface", "declaration", "swig_directive",
  "extend_directive", "$@1", "apply_directive", "clear_directive",
  "constant_directive", "echo_directive", "except_directive", "stringtype",
  "fname", "fragment_directive", "include_directive", "$@2", "includetype",
  "inline_directive", "insert_directive", "module_directive",
  "name_directive", "native_directive", "pragma_directive", "pragma_arg",
  "pragma_lang", "rename_directive", "rename_namewarn",
  "feature_directive", "stringbracesemi", "featattr", "varargs_directive",
  "varargs_parms", "typemap_directive", "typemap_type", "tm_list",
  "tm_tail", "typemap_parm", "types_directive", "template_directive",
  "warn_directive", "c_declaration", "$@3", "c_decl", "c_decl_tail",
  "initializer", "cpp_alternate_rettype", "cpp_lambda_decl",
  "lambda_introducer", "lambda_body", "lambda_tail", "$@4", "c_enum_key",
  "c_enum_inherit", "c_enum_forward_decl", "c_enum_decl",
  "c_constructor_decl", "cpp_declaration", "cpp_class_decl", "@5", "@6",
  "cpp_opt_declarators", "cpp_forward_class_decl", "cpp_template_decl",
  "$@7", "cpp_temp_possible", "template_parms", "templateparameters",
  "templateparameter", "templateparameterstail", "cpp_using_decl",
  "cpp_namespace_decl", "$@8", "$@9", "cpp_members", "$@10", "$@11",
  "$@12", "cpp_member", "cpp_constructor_decl", "cpp_destructor_decl",
  "cpp_conversion_operator", "cpp_catch_decl", "cpp_static_assert",
  "cpp_protection_decl", "cpp_swig_directive", "cpp_end", "cpp_vend",
  "anonymous_bitfield", "anon_bitfield_type", "storage_class", "parms",
  "rawparms", "ptail", "parm", "valparms", "rawvalparms", "valptail",
  "valparm", "def_args", "parameter_declarator",
  "typemap_parameter_declarator", "declarator", "notso_direct_declarator",
  "direct_declarator", "abstract_declarator", "direct_abstract_declarator",
  "pointer", "type_qualifier", "type_qualifier_raw", "type", "rawtype",
  "type_right", "decltype", "primitive_type", "primitive_type_list",
  "type_specifier", "definetype", "$@13", "default_delete",
  "deleted_definition", "explicit_default", "ename",
  "optional_constant_directive", "enumlist", "edecl", "etype", "expr",
  "valexpr", "exprnum", "exprcompound", "ellipsis", "variadic", "inherit",
  "raw_inherit", "$@14", "base_list", "base_specifier", "@15", "@16",
  "access_specifier", "templcpptype", "cpptype", "opt_virtual",
  "virt_specifier_seq", "exception_specification", "cpp_const", "ctor_end",
  "ctor_initializer", "mem_initializer_list", "mem_initializer",
  "template_decl", "identifier", "idstring", "idstringopt", "idcolon",
  "idcolontail", "idtemplate", "idcolonnt", "idcolontailnt", "string",
  "wstring", "stringbrace", "options", "kwargs", "stringnum", "empty", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   141,   142,   142,   142,   142,   142,   142,   142,   143,
     143,   144,   144,   144,   144,   144,   144,   144,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   145,
     145,   145,   145,   145,   145,   145,   145,   145,   145,   147,
     146,   148,   149,   150,   150,   150,   151,   151,   152,   152,
     152,   152,   153,   154,   154,   155,   155,   155,   157,   156,
     158,   158,   159,   159,   160,   160,   160,   160,   161,   162,
     162,   163,   163,   164,   164,   165,   165,   166,   166,   167,
     167,   167,   168,   168,   169,   169,   169,   169,   169,   169,
     169,   169,   170,   170,   170,   171,   171,   172,   173,   173,
     174,   174,   174,   175,   176,   177,   177,   178,   178,   178,
     179,   180,   181,   182,   182,   182,   183,   182,   182,   182,
     182,   184,   184,   185,   185,   185,   186,   186,   186,   186,
     187,   187,   187,   187,   187,   187,   187,   188,   188,   188,
     189,   190,   191,   192,   191,   193,   193,   193,   194,   194,
     195,   196,   196,   197,   198,   198,   198,   198,   198,   198,
     200,   199,   201,   199,   202,   202,   203,   205,   204,   204,
     204,   206,   206,   206,   206,   206,   206,   206,   207,   208,
     208,   209,   209,   210,   210,   211,   211,   213,   212,   214,
     212,   212,   215,   216,   217,   215,   215,   215,   218,   215,
     219,   219,   219,   219,   219,   219,   219,   219,   219,   219,
     219,   219,   219,   219,   219,   219,   219,   219,   219,   220,
     221,   221,   222,   222,   222,   222,   222,   223,   224,   225,
     225,   225,   226,   226,   226,   226,   226,   226,   226,   226,
     226,   226,   226,   227,   227,   227,   228,   228,   228,   229,
     230,   230,   230,   230,   230,   230,   231,   231,   231,   231,
     231,   231,   231,   231,   231,   231,   231,   231,   231,   231,
     231,   231,   232,   233,   233,   234,   234,   235,   235,   235,
     236,   237,   237,   238,   238,   239,   239,   240,   240,   240,
     240,   240,   241,   241,   241,   242,   242,   242,   243,   243,
     243,   243,   243,   243,   243,   243,   243,   243,   243,   243,
     243,   243,   243,   243,   243,   243,   243,   243,   243,   243,
     244,   244,   244,   244,   244,   244,   244,   244,   245,   245,
     245,   245,   245,   245,   245,   245,   245,   245,   246,   246,
     246,   246,   246,   246,   246,   246,   246,   246,   246,   246,
     246,   246,   247,   247,   247,   247,   247,   247,   247,   248,
     248,   248,   248,   249,   249,   250,   250,   250,   251,   252,
     252,   252,   252,   253,   253,   253,   253,   253,   253,   253,
     253,   253,   254,   255,   256,   256,   257,   257,   257,   257,
     257,   257,   257,   257,   257,   257,   257,   257,   257,   257,
     259,   258,   258,   260,   260,   261,   262,   263,   263,   264,
     264,   265,   265,   265,   265,   266,   266,   267,   268,   268,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   270,   270,
     270,   270,   270,   270,   270,   270,   271,   271,   271,   271,
     271,   271,   271,   271,   271,   271,   271,   271,   271,   271,
     271,   271,   271,   271,   271,   271,   271,   271,   272,   273,
     273,   274,   276,   275,   275,   277,   277,   279,   278,   280,
     278,   281,   281,   281,   282,   282,   282,   282,   283,   283,
     283,   284,   284,   285,   285,   285,   285,   286,   286,   286,
     286,   286,   287,   287,   287,   287,   288,   288,   288,   288,
     288,   288,   289,   289,   290,   290,   290,   290,   291,   291,
     292,   292,   293,   293,   293,   294,   294,   294,   295,   295,
     296,   296,   296,   296,   296,   296,   297,   297,   297,   297,
     298,   299,   299,   299,   299,   299,   299,   300,   300,   300,
     300,   301,   301,   302,   302,   303,   303,   303,   304,   304,
     305,   305,   305,   305,   305,   305,   306,   306,   307
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     2,     3,     2,     5,     3,     2,
       1,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       7,     5,     3,     5,     5,     3,     2,     2,     5,     2,
       5,     2,     4,     1,     1,     7,     7,     5,     0,     7,
       1,     1,     2,     2,     1,     5,     5,     5,     3,     4,
       3,     7,     8,     5,     3,     1,     1,     3,     1,     4,
       7,     6,     1,     1,     7,     9,     8,    10,     5,     7,
       6,     8,     1,     1,     5,     4,     5,     7,     1,     3,
       6,     6,     8,     1,     2,     3,     1,     2,     3,     6,
       5,     9,     2,     1,     1,     1,     0,     6,     1,     3,
       8,     5,     7,     1,     4,     1,     1,     2,     2,     3,
       1,     1,     1,     2,     1,     1,     1,    11,    13,     7,
       1,     1,     1,     0,     3,     1,     2,     2,     2,     1,
       5,     8,    10,     6,     1,     1,     1,     1,     1,     1,
       0,     9,     0,     8,     1,     3,     4,     0,     6,     3,
       4,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       1,     1,     1,     3,     1,     3,     4,     0,     6,     0,
       5,     5,     2,     0,     0,     7,     1,     1,     0,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     6,
       6,     7,     8,     8,     8,     9,     7,     5,     2,     2,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     4,     2,     2,     4,     2,     5,
       1,     1,     1,     2,     1,     1,     1,     2,     3,     1,
       1,     1,     1,     1,     1,     2,     1,     2,     2,     2,
       2,     1,     1,     2,     1,     3,     1,     2,     7,     3,
       1,     2,     1,     3,     1,     1,     1,     2,     5,     2,
       2,     1,     2,     2,     1,     1,     1,     1,     2,     3,
       3,     1,     2,     2,     3,     4,     5,     4,     5,     6,
       6,     4,     5,     5,     6,     7,     8,     8,     7,     7,
       1,     2,     3,     4,     5,     3,     4,     4,     1,     2,
       4,     4,     4,     5,     3,     4,     4,     5,     1,     2,
       2,     2,     3,     3,     1,     2,     2,     1,     1,     2,
       3,     4,     3,     4,     2,     3,     3,     4,     3,     3,
       2,     2,     1,     1,     2,     1,     1,     1,     1,     2,
       1,     2,     3,     1,     1,     1,     2,     2,     1,     1,
       2,     1,     4,     1,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     5,     3,     3,     1,     1,     3,     1,     1,     1,
       1,     1,     5,     8,     1,     1,     1,     1,     3,     4,
       5,     5,     5,     6,     6,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     2,     2,     2,     2,     2,     3,     1,
       1,     1,     0,     3,     1,     1,     3,     0,     4,     0,
       6,     1,     1,     1,     1,     1,     4,     4,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     1,     1,
       2,     4,     1,     1,     2,     1,     3,     3,     4,     4,
       3,     4,     2,     1,     1,     3,     4,     6,     2,     2,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     4,     1,     3,     1,     3,     3,     2,     2,     2,
       2,     2,     4,     1,     3,     1,     3,     3,     2,     2,
       2,     2,     1,     2,     1,     1,     1,     1,     3,     1,
       3,     5,     1,     3,     3,     5,     1,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     568,     0,     0,     0,     0,     0,    10,     4,   522,   386,
     394,   387,   388,   391,   392,   389,   390,   375,   393,   374,
     395,   568,   378,   396,   397,   398,   399,     0,   365,   366,
     367,   489,   490,   145,   484,   485,     0,   523,   524,     0,
       0,   534,     0,     0,     0,   363,   568,   370,   381,   373,
     383,   384,   488,     0,   568,   379,   532,     6,     0,     0,
     568,     1,    15,    64,    60,    61,     0,   260,    14,   256,
     568,     0,     0,    82,    83,   568,   568,     0,     0,   259,
     261,   262,     0,   263,   264,   266,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       9,    11,    18,    19,    20,    21,    22,    23,    24,    25,
     568,    26,    27,    28,    29,    30,    31,    32,     0,    33,
      34,    35,    36,    37,    38,    12,   113,   118,   115,   114,
      16,    13,   154,   155,   156,   157,   158,   159,     0,   271,
     568,   376,   521,     0,   147,   146,     0,     0,     0,     0,
       0,   377,     3,   369,   364,   568,     0,   400,     0,     0,
     534,   348,   347,   362,     0,   294,   277,   568,   301,   568,
     344,   338,   328,   291,   371,   385,   380,   540,     0,     0,
     530,     5,     8,     0,   272,   568,   274,    17,     0,   552,
     269,     0,   257,     0,     0,   559,     0,     0,   368,   568,
       0,     0,     0,     0,    78,     0,   568,   265,   268,   568,
     270,   267,     0,     0,   189,   568,     0,     0,    62,    63,
       0,     0,    51,    49,    46,    47,   568,     0,   568,     0,
     568,   568,     0,   112,   568,   568,     0,     0,     0,     0,
       0,     0,     0,   328,     0,   568,     0,   568,   554,   426,
     427,   438,   439,   440,   441,   442,   443,   444,   445,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   285,     0,
     280,   568,   419,   368,     0,   418,   420,   424,   421,   425,
     282,   279,     0,     0,     0,   535,   533,     0,   372,   568,
     348,   347,     0,     0,   338,   379,     0,   289,   405,   406,
     287,     0,   402,   403,   404,   354,     0,   418,   290,     0,
     568,     0,     0,   303,   346,   320,     0,   302,   345,   360,
     361,   329,   292,   568,     0,   293,   568,     0,     0,   341,
     340,   298,   339,   320,   349,   539,   538,   537,     0,     0,
     273,   276,   526,   525,     0,   527,     0,   551,   116,   258,
     562,     0,    68,    45,     0,   568,   400,    70,     0,     0,
       0,    74,     0,     0,     0,    98,     0,     0,   185,   119,
     568,     0,   187,     0,     0,   103,     0,     0,     0,   107,
     295,   296,   297,    42,     0,   104,   106,   528,     0,   529,
      54,     0,    53,     0,     0,   178,   568,   182,   488,   180,
     169,     0,     0,     0,     0,   525,     0,     0,     0,     0,
       0,     0,   320,     0,     0,   328,   568,   568,   408,   568,
     568,   472,     0,   471,   380,   474,     0,     0,     0,   436,
     435,   463,   464,   437,   466,   465,   520,     0,   281,   284,
     467,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   553,   486,
     487,   382,   531,     0,   348,   347,   338,   379,     0,   328,
       0,   358,   356,   341,   340,     0,   328,   349,     0,     0,
     401,   355,   568,   338,   379,     0,   321,   568,     0,     0,
     359,     0,   334,     0,     0,   352,     0,     0,     0,   300,
     343,     0,   299,   342,   350,     0,     0,     0,   304,   536,
       7,   568,     0,   170,   568,     0,     0,   558,     0,     0,
      69,    39,    77,     0,     0,     0,     0,     0,     0,     0,
     186,     0,     0,   568,   568,     0,     0,   108,     0,   568,
       0,     0,     0,     0,     0,   167,     0,   179,   184,    58,
       0,     0,     0,     0,    79,     0,     0,     0,     0,     0,
     149,     0,   379,     0,   498,   493,   494,     0,   126,   568,
     499,   568,   568,   162,   166,     0,   428,     0,     0,   362,
       0,   568,     0,   568,   461,   460,   458,   459,     0,   457,
     456,   452,   453,   451,   455,   454,   447,   446,   450,   449,
     448,     0,     0,   349,   332,   331,   330,   350,     0,   311,
       0,     0,     0,   320,   322,   349,     0,     0,   325,     0,
       0,   336,   335,   357,   353,     0,     0,     0,     0,     0,
       0,   305,   351,     0,     0,     0,   307,   275,    66,    67,
      65,     0,   563,   564,   567,   566,   560,    44,    43,     0,
      76,    73,    75,   557,    93,   556,     0,    88,   568,   555,
      92,     0,   566,     0,     0,    99,   568,   227,   190,   191,
       0,   256,     0,     0,    50,    48,   568,    41,   105,     0,
     545,   543,     0,    57,     0,     0,   110,     0,   568,   568,
     568,   568,     0,     0,   132,   131,   568,   134,   568,   136,
     130,   135,   140,     0,   148,   150,   568,   568,   568,     0,
     500,   496,   495,     0,   123,   125,   121,   127,   568,   128,
     491,   473,   475,   477,   492,     0,   160,   429,     0,     0,
     362,   361,     0,     0,     0,     0,     0,   283,     0,   568,
     333,   288,   337,   323,     0,   313,   327,   326,   312,   308,
       0,     0,     0,     0,     0,   306,     0,     0,     0,   117,
       0,     0,   198,   218,     0,     0,     0,     0,   261,     0,
       0,   240,   241,   233,   242,   216,   196,   238,   234,   232,
     235,   236,   237,   239,   217,   213,   214,   200,   208,   207,
     211,   210,     0,     0,   201,   202,   206,   212,   203,   204,
     205,   215,     0,   271,   568,   502,   503,     0,   505,     0,
       0,     0,     0,    90,   568,     0,   188,   257,     0,   568,
     101,     0,   100,     0,     0,     0,     0,   541,   568,     0,
      52,     0,   256,     0,   171,   172,   176,   175,   168,   173,
     177,   174,     0,   183,     0,     0,    81,   133,     0,   568,
     141,     0,   409,   414,     0,   410,   568,   400,   503,   568,
     153,     0,     0,   568,   129,   568,   482,   481,   483,     0,
     479,     0,     0,   432,   431,   430,     0,     0,   422,     0,
     462,   278,   324,   310,   309,     0,     0,     0,   314,     0,
       0,   565,   561,     0,   193,   230,   229,   231,     0,   228,
       0,    40,   192,   375,   374,   568,   378,     0,     0,     0,
     373,   379,     0,   504,    84,   566,    95,    89,   568,     0,
       0,    97,     0,    71,     0,   109,   546,   544,   550,   549,
     548,     0,    55,    56,     0,   568,     0,    59,    80,   122,
       0,   143,   142,   139,   568,   415,   568,     0,     0,     0,
       0,     0,     0,   513,   497,   501,     0,   476,   568,   568,
       0,     0,   434,   433,   568,   315,     0,     0,   319,   318,
     199,     0,     0,   568,   376,     0,     0,   568,   209,     0,
      96,     0,    91,   568,    86,    72,   102,   542,   547,     0,
     120,     0,   568,     0,   413,     0,   412,   151,   568,     0,
     510,     0,   512,   514,     0,   506,   507,   124,     0,   469,
     478,   470,     0,   164,   163,   568,     0,     0,   317,   316,
       0,   568,     0,   568,     0,     0,     0,     0,     0,    94,
      85,     0,   111,   167,     0,   144,   416,   417,   568,     0,
     508,   509,   511,     0,     0,   518,   519,     0,   568,     0,
     161,   423,   194,     0,   568,     0,   568,   568,   568,     0,
     249,   568,    87,     0,     0,   411,   152,   515,     0,   468,
     480,   165,     0,   568,   220,     0,   568,     0,     0,     0,
     568,   219,     0,   137,     0,   516,   195,   221,     0,   243,
     245,     0,   226,   568,   568,   568,     0,     0,     0,   246,
     248,   400,     0,   224,   223,   222,   568,   138,   517,     0,
     244,   225,   247
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,   100,   101,   102,   649,   771,   772,   773,
     774,   107,   390,   391,   775,   776,   690,   110,   111,   777,
     113,   778,   115,   779,   651,   203,   780,   118,   781,   657,
     526,   782,   364,   783,   374,   229,   385,   230,   784,   785,
     786,   787,   514,   126,   716,   567,   698,   127,   703,   851,
     943,   993,    42,   559,   128,   129,   130,   131,   788,   872,
     725,  1014,   789,   790,   688,   838,   394,   395,   396,   547,
     791,   136,   533,   370,   792,   971,  1072,   893,   793,   794,
     795,   796,   797,   798,   799,   800,  1074,  1087,   801,   908,
     802,   292,   184,   340,   185,   269,   270,   438,   271,   568,
     166,   379,   167,   313,   168,   169,   170,   242,    44,    45,
     272,   198,    47,    48,    49,    50,    51,   300,   301,   342,
     303,   304,   416,   853,   854,   944,  1036,   274,   307,   276,
     277,  1009,  1010,   422,   423,   572,   721,   722,   869,   959,
     870,    52,    53,   723,   570,   806,  1088,   860,   952,  1002,
    1003,   177,    54,   350,   388,    55,   180,    56,   682,   827,
     278,   279,   660,   194,   351,   646,   186
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -957
static const yytype_int16 yypact[] =
{
     686,  4260,  4332,   196,    66,  3750,  -957,  -957,  -957,  -957,
    -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,
    -957,   -68,  -957,  -957,  -957,  -957,  -957,    25,  -957,  -957,
    -957,  -957,  -957,   330,   122,   126,   182,  -957,  -957,   -18,
     110,  -957,    97,   222,  4977,   811,   763,   811,  -957,  -957,
    -957,  2891,  -957,    97,   -68,  -957,     2,  -957,   237,   247,
    4694,  -957,   185,  -957,  -957,  -957,   271,  -957,  -957,    47,
     297,  4404,   337,  -957,  -957,   297,   399,   466,   468,   382,
    -957,  -957,   489,  -957,  -957,   252,   272,   407,   496,   257,
     498,   547,   114,  4765,  4765,   506,   515,   562,   533,   180,
    -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,
     297,  -957,  -957,  -957,  -957,  -957,  -957,  -957,   988,  -957,
    -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,
    -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  4836,  -957,
    2182,  -957,  -957,   502,  -957,  -957,   532,   534,    97,   120,
     420,  -957,  -957,   811,  -957,  3374,   552,   313,  2310,  3078,
     444,  1257,  3245,   452,    97,  -957,  -957,   312,   381,   312,
     383,  1254,   500,  -957,  -957,  -957,  -957,  -957,   151,   173,
    -957,  -957,  -957,   550,  -957,   576,  -957,  -957,   471,  -957,
    -957,   420,    52,   471,   471,  -957,   581,  1660,  -957,   154,
    1029,    97,   151,   151,  -957,   471,  4622,  -957,  -957,  4694,
    -957,  -957,    97,   332,  -957,   195,   583,   151,  -957,  -957,
     471,   151,  -957,  -957,  -957,   624,  4694,   591,   264,   607,
     627,   471,   562,   624,  4694,  4694,    97,   562,  1461,  1314,
    1390,   471,   477,   559,  1660,    97,  1810,   135,  -957,  -957,
    -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  3078,
     363,  3078,  3078,  3078,  3078,  3078,  3078,  3078,  -957,   568,
    -957,   653,   665,   394,  2406,    23,  -957,  -957,   624,   712,
    -957,  -957,   673,   676,   687,  -957,     2,   636,  -957,  3491,
     198,   198,   700,   704,  1695,   631,   707,  -957,  -957,  -957,
     705,  3078,  -957,  -957,  -957,  -957,  4408,  -957,  2406,   724,
    3491,   719,    97,   401,   383,  -957,   720,   401,   383,  -957,
     638,  -957,  -957,  4694,  2438,  -957,  4694,  2566,   728,  3251,
    3257,   401,   383,   668,  1844,  -957,  -957,     2,   743,  4694,
    -957,  -957,  -957,  -957,   748,   624,    97,  -957,  -957,  -957,
     331,   751,  -957,  -957,   654,   312,   499,  -957,   757,   754,
     764,   762,   614,   776,   783,  -957,   784,   794,  -957,  -957,
    -957,    97,  -957,   786,   797,  -957,   799,   801,  4765,  -957,
    -957,  -957,  -957,  -957,  4765,  -957,  -957,  -957,   804,  -957,
    -957,   637,    55,   806,   744,  -957,   809,  -957,    74,  -957,
    -957,   200,   261,   261,   261,   346,   742,   826,    60,   834,
    2264,  2392,   752,  1844,   768,    31,   805,   349,  -957,  3563,
    1634,  -957,   838,  -957,    53,  -957,  2149,  4907,   837,  3027,
    2156,  -957,  -957,  -957,  -957,  -957,  -957,  2182,  -957,  -957,
    -957,  3078,  3078,  3078,  3078,  3078,  3078,  3078,  3078,  3078,
    3078,  3078,  3078,  3078,  3078,  3078,  3078,  3078,  -957,  -957,
    -957,  -957,  -957,   420,   484,   484,  1157,   771,   372,  -957,
     464,  -957,  -957,   484,   484,   487,   774,   261,   261,  3078,
    2406,  -957,  4694,  1866,    19,   852,  -957,  4694,  2694,   858,
    -957,   876,  -957,  4769,   881,  -957,  4856,   879,   880,   401,
     383,   883,   401,   383,  1823,   888,   890,  2520,   401,  -957,
    -957,   576,   296,  -957,  -957,   471,  2025,  -957,   886,   896,
    -957,  -957,  -957,   403,  1252,  2006,   901,  4694,  1660,   878,
    -957,  3852,   902,  -957,   825,  4765,   389,   909,   904,   627,
     279,   911,   471,  4694,    63,   864,  4694,  -957,  -957,  -957,
     261,   220,   851,    43,  -957,  1879,  5047,   903,  4977,   436,
    -957,   921,   742,   923,   189,   884,   887,   745,  -957,   556,
    -957,   312,   908,  -957,  -957,   922,  3078,  2822,  2950,  3206,
      -1,   763,   931,   653,   699,   699,   889,   889,  2279,  2661,
    3027,  3220,  2669,  2156,   780,   780,   739,   739,  -957,  -957,
    -957,    97,   774,  -957,  -957,  -957,  -957,   484,   508,   381,
    4981,   946,   544,   774,  -957,   851,   851,   947,  -957,  4993,
     851,  -957,  -957,  -957,  -957,   851,   942,   953,   959,   960,
    2648,   401,   383,   961,   962,   963,   401,  -957,  -957,  -957,
     624,  3954,  -957,   952,  -957,    55,   970,  -957,  -957,  2054,
    -957,  -957,   624,  -957,  -957,  -957,   973,  -957,  1100,   624,
    -957,   976,    49,   693,  1252,  -957,  1100,  -957,  -957,  -957,
    4056,    46,  4907,   400,  -957,  -957,  4694,  -957,  -957,   874,
    -957,    78,   913,  -957,   993,   992,  -957,    97,  1061,   809,
    -957,  1100,   287,   851,  -957,  -957,   -68,  -957,  1634,  -957,
    -957,  -957,  -957,   441,  -957,  -957,   955,  1360,  4694,  3078,
    -957,  -957,  -957,  1660,  -957,  -957,  -957,  -957,   312,  -957,
    -957,   998,  -957,   821,  -957,  2054,  -957,  2406,  3078,  3078,
    3206,  3633,  3078,  1000,  1001,  1002,  1005,  -957,  3078,   312,
    -957,  -957,  -957,  -957,   554,   401,  -957,  -957,   401,   401,
     851,   851,   999,  1003,  1007,   401,   851,  1008,  1009,  -957,
     471,   471,  -957,  -957,  1012,   977,   983,   984,   924,  1006,
     151,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,
    -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,  -957,
    -957,  -957,  1016,  2054,  -957,  -957,  -957,  -957,  -957,  -957,
    -957,  -957,  4475,  1022,  4694,   733,  -957,    63,  -957,  2025,
    1151,   471,  1034,  -957,  1100,  1028,  -957,    83,  1660,   141,
    -957,  4765,  -957,  1038,   255,   151,   375,  -957,  2182,   270,
    -957,   995,    47,   404,  -957,  -957,  -957,  -957,  -957,  -957,
    -957,  -957,  4547,  -957,  4158,  1041,  -957,  -957,   745,  4694,
    -957,   475,  -957,   151,   510,  -957,  4694,   499,  1031,  1010,
    -957,  1046,  2771,  1634,  -957,   908,  -957,  -957,  -957,    97,
    -957,  1044,  2054,  2406,  2406,  2406,  3078,  3078,  -957,  4907,
    2534,  -957,  -957,   401,   401,   851,  1049,  1050,   401,   851,
     851,  -957,  -957,  2054,  -957,  -957,  -957,  -957,   151,  -957,
    1052,  -957,  -957,  1024,  1025,   -68,  1026,  4907,  1033,  1828,
    1036,   265,  1063,  -957,  -957,   624,  1068,  -957,  1100,  1372,
      63,  -957,  1070,  -957,  1072,  -957,  -957,    78,  -957,  -957,
      78,  1017,  -957,  -957,   151,  4694,  1660,  -957,  -957,  -957,
    1082,  -957,  -957,  -957,   955,  1071,   955,  1489,  1091,  1090,
     499,    97,   543,  -957,  -957,  -957,   745,  -957,  1088,   908,
    1617,  1095,  2406,  2406,   763,   401,   851,   851,   401,   401,
    -957,  2054,  1097,  4694,  1076,    64,  3078,  3563,  -957,  1105,
    -957,  1110,  -957,  1100,  -957,  -957,  -957,  -957,  -957,  1113,
    -957,  1058,  1100,  1124,  -957,  3078,   151,  -957,  1634,   592,
    -957,  1125,  1128,  1123,   470,  -957,  -957,  -957,  1129,  -957,
    -957,  -957,    97,  -957,  -957,  1634,  1617,  1131,   401,   401,
    1126,  4694,  1135,  4694,  1138,  1142,   -17,  2899,  1144,  -957,
    -957,  1150,  -957,  -957,     4,  -957,  -957,  2406,   955,   745,
    -957,  -957,  -957,    97,  1139,  -957,  -957,  1148,  1088,   745,
    -957,  -957,  -957,  1154,  1100,  1159,  4694,  4694,  4694,  1163,
    -957,  1360,  -957,  4907,   475,  -957,  -957,  1156,  1158,  -957,
    -957,  -957,  2054,  1100,  -957,   588,  1100,  1164,  1166,  1170,
    4694,  -957,  1169,  -957,  1168,  -957,  -957,  -957,   605,  -957,
    -957,   499,  -957,  1100,  1100,  1100,  1176,   475,  1172,  -957,
    -957,   499,  1178,  -957,  -957,  -957,  1100,  -957,  -957,  1182,
    -957,  -957,  -957
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -957,  -957,  -350,  -957,  -957,  -957,  -957,    32,    39,     6,
      44,  -957,   711,  -957,    67,    76,  -957,  -957,  -957,    81,
    -957,    82,  -957,    86,  -957,  -957,    88,  -957,    89,  -531,
    -635,    90,  -957,   112,  -957,  -331,   689,   -85,   129,   131,
     134,   136,  -957,   545,  -812,  -672,  -957,  -957,  -957,  -956,
    -683,  -957,  -119,  -957,  -957,  -957,  -957,  -957,    12,  -957,
    -957,   216,    22,    27,  -957,  -957,   299,  -957,   690,   548,
     140,  -957,  -957,  -957,  -701,  -957,  -957,  -957,  -957,   551,
    -957,   555,   156,   557,  -957,  -957,  -957,  -467,  -957,  -957,
      11,   -54,  -957,   727,    13,   414,  -957,   661,   814,    -7,
    -580,  -957,   -39,  1147,  -183,  -107,    -5,    18,    35,  -957,
     -64,     8,   -11,   696,  -525,  1203,  -957,  -338,  -957,  -154,
    -957,  -957,  -957,  -887,  -957,   263,  -957,  1192,  -115,  -482,
    -957,  -957,   215,   840,  -957,  -957,  -957,   409,  -957,  -957,
    -957,  -231,   -48,   310,   714,  -375,  -561,   214,  -957,  -957,
     236,   -19,   981,  -117,  -957,   919,  -224,  -137,  -957,  -266,
    1030,  -957,   608,   209,  -182,  -503,     0
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -569
static const yytype_int16 yytable[] =
{
       6,   735,   141,   302,   398,   139,   183,   197,   227,    46,
      46,   105,   286,   686,    43,    58,   138,   132,   519,   245,
     531,   142,   663,  1058,   871,   275,   848,   133,   812,   228,
     228,   700,   134,   153,   644,   140,   939,   103,   375,   165,
     732,   335,   337,   644,   104,   571,   173,   538,   293,   106,
     850,   236,   189,   189,   142,   347,   407,   994,   347,   996,
     614,   347,   462,     8,   171,  -286,    61,   653,    46,   189,
     195,   344,   108,   143,   246,   195,   204,   352,  1064,   241,
     154,   109,   174,   358,   614,   557,   112,   114,   362,   347,
     247,   116,   902,   117,   119,   120,   574,   807,   348,  -568,
       8,   543,   287,  -527,  1023,   815,   654,   468,   470,   655,
     195,   475,  1059,   509,   387,  1063,  -181,   121,   224,   178,
     189,   381,   149,     8,   409,   733,  1097,  -286,   734,   421,
     845,   190,   190,   813,   122,   615,   123,   349,     8,   124,
     280,   125,   179,   346,  1007,   135,   859,   413,   273,    37,
      38,  1065,   365,   268,     8,   366,   314,   318,   355,   693,
     322,   137,   325,    46,   641,   191,   332,   173,   349,   173,
     146,   961,   377,   294,   147,    39,     8,   656,  -181,    41,
     393,   319,   293,   670,   923,   341,    37,    38,   288,   380,
    1024,   956,   970,  1025,   718,   825,   163,    59,   320,   142,
     163,     8,   302,   293,   673,   414,   347,   420,   356,    37,
      38,   421,    39,   150,    46,   142,    41,    46,   826,   468,
     470,   475,   148,     8,    37,    38,   189,  1066,   382,   709,
     386,   389,    31,    32,    46,   399,    60,  1071,   155,   285,
      37,    38,    46,    46,   140,   418,   171,   425,   397,   371,
      39,    34,    35,   920,    41,   158,   404,   140,     8,   549,
     354,   218,    37,    38,     8,   152,   165,     8,   156,   491,
    1020,   439,   494,   173,   932,     8,   914,   910,   565,   566,
     181,   980,     8,   235,   201,   314,   318,    37,    38,   332,
     182,   171,   336,   347,   608,   609,   210,    46,   140,   539,
     638,   354,   189,   219,   155,   187,   916,   466,   978,    37,
      38,   188,   156,    39,   228,   398,   933,   160,    46,   237,
     228,   158,   275,   571,   500,   503,  1039,   644,   483,   211,
     846,    46,   858,   642,    46,    39,   164,   193,   490,   160,
     844,  -255,   639,  1049,    37,    38,   239,    46,   518,   240,
      37,    38,   511,    37,    38,   173,   163,   981,   164,   297,
     684,    37,    38,   581,   212,   561,   157,   608,    37,    38,
       6,  1086,   404,   515,   926,   368,    39,   200,     8,    39,
     160,  1083,   144,   160,  1017,   516,   369,    39,   159,   984,
     161,    41,  -407,   162,   679,  -407,   548,     8,   680,   164,
     163,   145,   164,   427,   653,   142,   189,   650,   661,   189,
       8,   428,   323,   604,  1107,   601,   560,   142,   298,   299,
     173,   323,  1031,   326,   425,  -407,   552,    46,   611,   324,
     913,  1034,   674,   617,   155,   675,  -568,   404,   324,   202,
     327,   487,   156,   820,   580,   273,   655,   309,   157,   140,
     268,   158,   140,   214,   821,   569,    31,    32,   488,   314,
     318,   332,  -568,  -568,    37,    38,   207,   208,   500,   503,
     159,   228,    31,    32,     8,    34,    35,   189,   332,   705,
       8,   849,   706,    37,    38,   658,  -568,   850,   571,   666,
      46,    34,    35,  1075,   929,    46,    37,    38,  -568,   632,
     859,    28,    29,    30,   323,   605,   205,   935,   206,    39,
    1045,   341,   691,   160,     6,   941,  1046,   408,   942,   949,
     161,   324,    39,   162,   289,   328,    41,   323,   606,   209,
     163,   139,   164,     6,   139,    46,   217,   105,   220,   386,
     665,   158,   138,   132,   324,   672,   231,   704,   323,   740,
     281,    46,   946,   133,    46,   232,   685,   947,   134,   397,
      37,    38,   717,   103,   719,   324,    37,    38,   189,   173,
     104,   173,   724,   234,   165,   106,   298,   299,   891,   892,
     282,   173,   283,   439,   487,   743,  1005,   221,   163,  1006,
     222,   338,    39,   223,   487,   882,    41,   319,   108,   171,
     296,   488,   632,   410,   298,   299,   411,   109,   818,  1092,
     157,   488,   112,   114,   731,   312,   334,   116,   339,   117,
     119,   120,   823,   571,   353,   814,  1103,  1104,  1105,   372,
     347,  1089,   159,   900,  1090,  1040,   563,   378,  1041,  1111,
     571,   139,  1091,   121,   564,   565,   566,   105,  1099,   803,
     383,  1100,   138,   132,   861,   524,   525,     8,   808,  1101,
     122,   987,   123,   133,   988,   124,   808,   125,   134,   384,
     139,   135,   436,   103,   863,   413,   105,   847,   541,   542,
     104,   138,   132,   245,    46,   106,   858,   137,   139,   548,
       6,   808,   133,   805,   661,   437,   142,   134,   173,   842,
     835,   805,   103,   302,   398,   440,   855,   808,   108,   104,
     836,   864,   852,   275,   106,   837,    46,   109,   173,   458,
     153,   459,   112,   114,   460,   803,   805,   116,   461,   117,
     119,   120,   881,   569,   810,   811,   924,   108,   909,   173,
     463,   471,   805,    37,    38,   472,   109,   477,   319,   490,
     912,   112,   114,   121,   247,   478,   116,   228,   117,   119,
     120,   972,   479,  1109,   482,   731,     8,   485,   489,    39,
     122,   918,   123,    41,   163,   124,   497,   125,   909,   922,
     402,   135,   121,   403,   504,   236,   510,   713,   714,   512,
     163,   715,   517,   803,   247,   940,  1001,   137,   520,   122,
     521,   123,   948,   155,   124,   522,   125,     1,     2,     3,
     135,   156,    46,   563,   808,   964,   523,   157,   527,   142,
     158,   564,   565,   566,   528,   529,   137,   534,   280,   451,
     452,   453,   454,   455,   456,   457,   273,   530,   535,   159,
     536,   268,   537,   975,   139,   540,    67,   544,   545,   805,
     105,   546,    37,    38,     8,   138,   132,    46,   550,   953,
      28,    29,    30,   173,    46,   724,   133,   551,   555,   671,
     420,   134,   803,   455,   456,   457,   103,   554,    39,   556,
     983,   558,   160,   104,   573,   582,   974,   603,   106,   161,
     607,   408,   162,   803,   866,   867,   868,   414,   569,   163,
     616,   164,    79,    80,    81,   142,   620,    83,   998,    84,
      85,   108,   453,   454,   455,   456,   457,   621,   808,  1022,
     109,  1015,   623,  1028,   667,   112,   114,   625,   626,   647,
     116,   627,   117,   119,   120,   399,   633,  1102,   634,   648,
      37,    38,   664,    46,   855,   669,   855,   302,   397,   676,
     852,   677,   852,   805,   683,   687,   121,   165,  1011,   724,
     702,   151,   707,   708,   173,   172,    39,  1053,   726,  1055,
      41,   803,   176,   122,   711,   123,   712,  1015,   124,   736,
     125,    46,   171,   808,   135,    46,   720,   742,   746,   312,
     750,     8,   808,  1026,   760,   404,   441,   442,   173,  1082,
     137,   751,  1077,  1078,  1079,   213,   216,   752,   753,   756,
     757,   758,   761,   804,   824,   173,   828,    71,   805,   451,
     452,   453,   454,   455,   456,   457,  1096,   805,   238,    46,
     809,    46,     8,   569,   829,   189,   156,   243,   855,   830,
     865,   876,   877,   878,   852,   879,   899,   885,  1011,   934,
     569,   886,   199,   895,   808,   887,   889,   890,   894,   896,
     897,   808,   898,   901,    46,    46,    46,   284,   215,  -197,
     357,   921,   803,   808,   295,   919,   808,    37,    38,   925,
     315,   315,    67,   321,   938,   950,   951,   954,    46,   805,
     333,   960,   973,   808,   808,   808,   805,   966,   967,   192,
    -252,  -251,  -254,    39,   979,   832,   808,   160,   805,   976,
     811,   805,  -250,   985,   239,   986,   243,   240,    37,    38,
     359,   989,   225,   992,   163,   995,   164,   233,   805,   805,
     805,   367,   999,  1000,   298,   299,  1008,  1021,    79,    80,
      81,   805,  1016,    83,   769,    84,    85,   172,  1029,    28,
      29,    30,  -253,  1030,     8,   400,  1032,   406,   315,   315,
       8,   412,  1033,   415,   151,   243,   424,  1035,  1042,   343,
    1043,  1044,  1051,  1052,   343,   343,  1054,  1047,  1056,   833,
     563,   343,  1057,   360,   361,  1061,   343,  1068,   564,   565,
     566,   354,   172,  1062,   917,  1073,  1069,   289,   373,   156,
    1076,   343,   376,  1080,  1084,  1093,  1085,  1094,   467,   469,
     469,  1095,   343,   476,   158,   850,  1098,  1106,   345,   405,
    1108,  1110,   343,   345,   345,  1112,   417,   643,   678,   484,
     345,   486,  1050,   834,   991,   345,   689,   843,   637,   839,
      37,    38,   931,   840,   737,   841,    37,    38,   315,   315,
     345,   583,   699,   315,   175,     8,   653,     8,   189,  1038,
       8,   345,   392,  1070,   575,   513,    39,   401,   345,  1012,
     160,   345,    39,   406,   957,  1081,    41,   239,   710,  1067,
     240,   822,     0,   473,     0,     0,   474,   163,     0,   164,
     532,     0,   354,     0,   310,   654,     0,   310,   655,     0,
     156,     0,   328,     0,     0,   311,     0,     0,     0,   317,
       0,   158,     0,     0,   158,     0,     0,     8,   331,     0,
       0,   469,   469,   469,     0,     0,     0,   553,     0,   315,
     315,     0,   315,     0,     0,     0,     0,     0,   562,     0,
       0,    37,    38,    37,    38,     0,    37,    38,     0,     0,
     306,   308,     0,     0,   408,     0,     0,     0,     0,     0,
       0,     0,   311,     0,     0,     0,   656,    39,     0,    39,
       0,   160,    39,    41,     0,     8,    41,     0,   239,     0,
     329,   240,     0,   330,     0,   602,     0,   317,   163,   331,
     164,     0,   312,     8,     0,   312,   469,   469,     0,     0,
     856,     0,   613,    37,    38,     0,     0,     0,     0,    28,
      29,    30,   354,     0,   857,   982,     0,     0,     0,     0,
     156,     0,     0,   315,     0,     0,   315,     0,     0,    39,
     408,     0,     0,    41,     0,     0,     0,     0,   316,     0,
     563,     0,     0,   243,     0,     0,     0,   243,   564,   565,
     566,   426,   312,   429,   430,   431,   432,   433,   434,   435,
       0,    37,    38,     0,     8,     0,     0,   189,     0,   469,
     243,   315,     0,     0,   315,   701,   499,   502,     0,    37,
      38,   508,     0,     0,     0,     0,     0,    39,     0,     0,
       0,   160,     8,   480,     0,     0,   343,     0,   239,     0,
     172,   240,     0,     0,     0,    39,   343,     0,   163,    41,
     164,     0,     0,     0,     0,     0,   493,     0,     0,   496,
     739,   681,     0,   343,     0,     0,     0,     0,   312,   354,
       0,     0,   997,     0,   315,   315,     0,   156,     0,   315,
       0,     0,   640,     0,   315,   345,   645,     0,     0,   315,
      37,    38,     0,   652,   659,   662,     0,   499,   502,     0,
     508,     0,     0,     0,     0,     0,   298,   299,     0,     0,
       0,     0,   345,     0,   659,     0,    39,     0,    37,    38,
      41,   692,     0,   243,     0,     0,     0,   402,     0,     0,
     403,     0,     0,     0,     0,     0,     0,   163,     0,     0,
       0,     0,     0,     0,    39,     0,   831,     0,   160,     0,
       0,     0,   315,     0,     0,   239,     0,     0,   240,     0,
       8,     0,     0,     0,     0,   163,     0,   164,     0,     0,
     612,     0,   243,   584,   585,   586,   587,   588,   589,   590,
     591,   592,   593,   594,   595,   596,   597,   598,   599,   600,
       0,   631,     0,   819,   636,     0,     0,   354,     0,     0,
    1013,     0,     0,     8,     0,   156,     0,     0,     0,   315,
     315,   610,     0,     0,     0,   315,     0,     0,     0,     0,
     619,     0,     0,    28,    29,    30,     0,     0,   157,     0,
       0,     0,     0,     0,   659,     0,     0,     0,     8,   612,
     354,   817,   631,   659,     0,     0,    37,    38,   156,     0,
     159,     0,     0,     0,   563,     0,     0,     0,     0,     0,
       0,   911,   564,   565,   566,     0,     0,     0,     0,   243,
       0,     0,    39,     0,     0,   155,   160,   243,     0,     0,
       0,   343,   343,   239,     0,     0,   240,     0,     0,    37,
      38,     0,   158,   163,     0,   164,     0,     0,     0,     0,
       0,     0,   744,   745,     0,     0,     0,   748,   727,   590,
     593,   600,   749,     0,     0,    39,     0,   755,     0,   160,
       0,     0,     0,     0,    37,    38,   239,     0,   958,   240,
     345,   345,   343,     0,     0,     0,   163,     0,   164,     0,
       0,     0,     0,     0,   315,   927,   928,   930,   315,   315,
      39,     0,     0,     8,   160,     0,     0,     0,     0,     0,
       0,   473,     0,     0,   474,     0,     8,     0,   243,     0,
       0,     8,     0,   164,   945,     0,     0,   659,   243,   915,
     744,   345,     0,     0,     0,     0,     0,     8,     0,     0,
     419,     0,     0,     0,     0,   243,     0,     0,   156,     0,
       0,     0,   817,   310,     0,     0,   243,     0,   977,     8,
    1004,   628,     0,     0,     0,     0,   156,     0,     0,   243,
     158,     0,     8,   172,   408,   315,   315,     0,     0,     0,
       0,     0,   505,     0,     0,     0,   562,   883,   884,    37,
      38,   862,     0,   888,     0,     0,   310,     0,     0,     0,
       0,     0,    37,    38,     0,   990,     0,    37,    38,   408,
     873,   874,   433,   158,   875,    39,     0,   628,     0,   160,
     880,  1048,     0,    37,    38,   243,   239,     0,    39,   240,
       0,     0,    41,    39,     0,     0,   163,   160,   164,   629,
     659,     0,   630,     0,   239,    37,    38,   240,     0,    39,
       0,   312,  1004,    41,   163,     0,   164,     0,    37,    38,
     506,     0,     0,   507,     0,     0,     0,   945,     0,     0,
       0,    39,   312,     0,     0,    41,     0,     0,     0,     0,
       0,     0,   473,     0,    39,   474,     0,     0,    41,     0,
       0,     0,     0,     0,   312,   629,     0,     0,   630,     8,
       0,     0,   189,     0,     0,     0,     0,   312,     0,   251,
     252,   253,   254,   255,   256,   257,   258,     0,     0,     0,
       0,   189,   965,     0,     0,     0,   968,   969,   251,   252,
     253,   254,   255,   256,   257,   258,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   762,     0,  -568,    63,     0,
       0,     0,    64,    65,    66,     0,     0,     0,   962,   963,
       0,     0,     0,     0,     0,    67,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,     0,    37,    38,   763,    69,     0,
       0,  -568,     0,  -568,  -568,  -568,  -568,  -568,     0,     0,
       0,   298,   299,  1018,  1019,     0,    71,    72,    73,    74,
     764,    76,    77,    78,  -568,  -568,  -568,   765,   766,   767,
       0,    79,   768,    81,     0,    82,    83,   769,    84,    85,
    -568,  -568,     0,  -568,  -568,    86,     0,     0,     0,    90,
       0,    92,    93,    94,    95,    96,    97,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    98,  1027,  -568,
       0,     0,    99,  -568,  -568,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     8,     0,  1037,   189,   248,
     576,     0,   770,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,     0,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   259,     0,     0,     0,     0,     0,     0,     0,
      27,    28,    29,    30,    31,    32,     0,   260,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,     0,   441,   442,   443,   444,
       0,   445,     0,   441,   442,   443,   444,     8,    36,     0,
       0,    37,    38,     0,   446,   577,   448,   449,   578,   451,
     452,   453,   454,   455,   456,   579,   451,   452,   453,   454,
     455,   456,   457,     0,     0,     0,     0,    39,     0,     0,
      40,    41,     0,     0,   408,     0,     0,     0,   261,     0,
       0,   262,   498,     8,   263,   264,   189,   248,   265,   266,
     267,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,     0,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     259,     0,     0,    37,    38,   738,     0,     0,     0,    28,
      29,    30,    31,    32,     0,   260,     0,     0,   305,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    39,
      33,    34,    35,    41,     0,     0,   441,   442,   443,   444,
       0,   445,     0,     0,     0,     8,    36,     0,     0,    37,
      38,     0,   312,     0,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    39,     0,     0,     0,    41,
       0,     0,   408,     0,     0,     0,   261,     0,     0,   262,
     501,     8,   263,   264,   189,   248,   265,   266,   267,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,     0,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   259,     0,
       0,    37,    38,     0,     0,     0,     0,    28,    29,    30,
      31,    32,     0,   260,     0,     0,   492,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    39,    33,    34,
      35,    41,     0,   441,   442,   443,   444,     0,   445,     0,
       0,     0,     0,     8,    36,     0,     0,    37,    38,     0,
     312,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    39,     0,     0,     0,    41,     0,     0,
     408,     0,     0,     0,   261,     0,     0,   262,   635,     8,
     263,   264,   189,   248,   265,   266,   267,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,     0,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   259,     0,     0,    37,
      38,     0,     0,     0,     0,    28,    29,    30,    31,    32,
       0,   260,     0,     0,   495,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    39,    33,    34,    35,    41,
       0,   441,   442,   443,   444,     0,     0,     0,     0,     0,
       0,     8,    36,     0,     0,    37,    38,     0,   312,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    39,     0,     0,     0,    41,     0,     0,   408,     0,
       0,     0,   261,     0,     0,   262,   754,     8,   263,   264,
     189,   248,   265,   266,   267,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,     0,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   259,     0,     0,    37,    38,     0,
       0,     0,     0,    28,    29,    30,    31,    32,     0,   260,
       0,     0,   618,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    39,    33,    34,    35,    41,   441,   442,
     443,   444,     0,     0,     0,     0,   441,   442,   443,   444,
      36,     0,     0,    37,    38,     0,   312,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   450,   451,
     452,   453,   454,   455,   456,   457,     0,     0,     0,    39,
       0,     0,   955,    41,     0,     0,     0,     0,     0,     0,
     261,     0,     0,   262,     0,     8,   263,   264,   189,   248,
     265,   266,   267,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,     0,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,   259,   728,     0,     0,     0,     0,     0,     0,
       0,    28,    29,    30,    31,    32,     0,   260,   441,   442,
     443,   444,     0,   445,     0,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,     0,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,    36,     0,
       0,    37,    38,     9,    10,    11,    12,    13,    14,    15,
      16,     0,    18,     0,    20,     0,     0,    23,    24,    25,
      26,     0,     0,     0,     0,     0,     0,    39,     0,     0,
       0,    41,  1060,     0,     0,     0,     0,     0,   261,     0,
       0,   262,     0,     8,   263,   264,   189,   248,   265,   266,
     267,   249,   250,   251,   252,   253,   254,   255,   256,   257,
     258,     0,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
     259,   729,     0,     0,     0,     0,     0,     0,     0,    28,
      29,    30,    31,    32,     0,   260,   441,   442,   443,   444,
       0,   445,     0,     0,     0,     0,     0,     0,     0,     0,
      33,    34,    35,     0,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,    36,     0,     0,    37,
      38,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    39,     0,     0,     0,    41,
       0,     0,     0,     0,     0,     0,   261,     0,     0,   262,
       0,     8,   263,   264,   189,   248,   265,   266,   267,   249,
     250,   251,   252,   253,   254,   255,   256,   257,   258,     0,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,   259,     0,
       0,     0,     0,     0,     0,     0,     0,    28,    29,    30,
      31,    32,     0,   260,   441,   442,   443,   444,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    33,    34,
      35,     0,     0,     0,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,    36,     0,     0,    37,    38,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    39,     0,     0,     0,    41,     0,     0,
       0,     0,     0,     0,   261,     0,     0,   262,     0,     8,
     263,   264,   189,   248,   265,   266,   267,   249,   250,   251,
     252,   253,   254,   255,   256,   257,   258,     0,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   259,     0,     8,     0,
       0,     0,     0,     0,     8,    28,    29,    30,    31,    32,
       8,   260,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    33,    34,    35,     0,
       0,     0,     0,     0,     0,   310,     0,     0,     0,     0,
       0,   310,    36,   316,     0,    37,    38,   310,     0,   498,
       0,     0,   158,     0,     0,   501,     0,     0,   158,     0,
       0,     0,     0,     0,   158,     0,     0,     0,     0,     0,
       0,    39,     0,     0,     0,    41,     0,   441,   442,   443,
     444,     0,     0,     0,    37,    38,     0,     0,   263,   264,
      37,    38,   730,   266,   267,     0,    37,    38,   449,   450,
     451,   452,   453,   454,   455,   456,   457,     0,     0,     0,
      39,     0,     0,     0,    41,     0,    39,     0,     0,     0,
      41,     0,    39,     0,     0,     0,    41,     8,     0,     0,
       0,     0,     0,   312,     0,     0,     0,     0,     0,   312,
       0,     0,     0,     0,     0,   312,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,   289,     0,     0,     0,     0,     0,
       0,     0,    27,    28,    29,    30,    31,    32,     0,     0,
       0,   158,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    33,    34,    35,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,    37,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    39,
       0,     0,    40,    41,     8,     0,     0,     0,     0,     0,
     290,     0,     0,   291,     0,     0,     0,     0,     0,     0,
     163,     0,     0,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,   289,     0,     0,     0,     0,     0,     0,     0,    27,
      28,    29,    30,    31,    32,     0,     0,     0,   158,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    33,    34,    35,     0,     0,     8,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    36,     0,     0,
      37,    38,     0,     0,     0,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     0,     0,     0,    39,     0,     0,    40,
      41,    27,    28,    29,    30,    31,    32,   464,     0,     0,
     465,     0,     0,     0,     0,     0,     0,   163,     0,     0,
       0,     0,     0,    33,    34,    35,     8,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,     0,    37,    38,     0,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,     0,     0,     0,     0,     0,    39,     0,
       0,    40,    41,     0,     0,    31,    32,     0,     0,   402,
       0,     0,   403,     0,     0,     0,     0,     0,     0,   163,
       0,     0,     0,    33,    34,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    36,
       0,     0,    37,    38,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    39,     0,
      -2,    62,    41,  -568,    63,     0,     0,     0,    64,    65,
      66,     0,     0,     0,     0,     0,     0,     0,     0,   163,
       0,    67,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,
       0,     0,     0,    68,    69,     0,     0,     0,     0,  -568,
    -568,  -568,  -568,  -568,     0,     0,    70,     0,     0,     0,
       0,     0,    71,    72,    73,    74,    75,    76,    77,    78,
    -568,  -568,  -568,     0,     0,     0,     0,    79,    80,    81,
       0,    82,    83,     0,    84,    85,  -568,  -568,     0,  -568,
    -568,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    62,     0,  -568,    63,     0,     0,     0,
      64,    65,    66,    98,     0,  -568,     0,     0,    99,  -568,
       0,     0,     0,    67,  -568,  -568,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,
    -568,  -568,     0,     0,     0,    68,    69,     0,     0,   668,
       0,  -568,  -568,  -568,  -568,  -568,     0,     0,    70,     0,
       0,     0,     0,     0,    71,    72,    73,    74,    75,    76,
      77,    78,  -568,  -568,  -568,     0,     0,     0,     0,    79,
      80,    81,     0,    82,    83,     0,    84,    85,  -568,  -568,
       0,  -568,  -568,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    62,     0,  -568,    63,     0,
       0,     0,    64,    65,    66,    98,     0,  -568,     0,     0,
      99,  -568,     0,     0,     0,    67,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,     0,     0,     0,    68,    69,     0,
       0,   759,     0,  -568,  -568,  -568,  -568,  -568,     0,     0,
      70,     0,     0,     0,     0,     0,    71,    72,    73,    74,
      75,    76,    77,    78,  -568,  -568,  -568,     0,     0,     0,
       0,    79,    80,    81,     0,    82,    83,     0,    84,    85,
    -568,  -568,     0,  -568,  -568,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    62,     0,  -568,
      63,     0,     0,     0,    64,    65,    66,    98,     0,  -568,
       0,     0,    99,  -568,     0,     0,     0,    67,  -568,  -568,
    -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,  -568,  -568,     0,     0,     0,    68,
      69,     0,     0,   816,     0,  -568,  -568,  -568,  -568,  -568,
       0,     0,    70,     0,     0,     0,     0,     0,    71,    72,
      73,    74,    75,    76,    77,    78,  -568,  -568,  -568,     0,
       0,     0,     0,    79,    80,    81,     0,    82,    83,     0,
      84,    85,  -568,  -568,     0,  -568,  -568,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    62,
       0,  -568,    63,     0,     0,     0,    64,    65,    66,    98,
       0,  -568,     0,     0,    99,  -568,     0,     0,     0,    67,
    -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,
    -568,  -568,  -568,  -568,  -568,  -568,  -568,  -568,     0,     0,
       0,    68,    69,     0,     0,     0,     0,  -568,  -568,  -568,
    -568,  -568,     0,     0,    70,     0,     0,     0,   937,     0,
      71,    72,    73,    74,    75,    76,    77,    78,  -568,  -568,
    -568,     0,     0,     0,     0,    79,    80,    81,     0,    82,
      83,     0,    84,    85,  -568,  -568,     0,  -568,  -568,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,     7,     0,     8,     0,     0,     0,     0,     0,     0,
       0,    98,     0,  -568,     0,     0,    99,  -568,     0,     0,
       0,     0,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
       0,     0,     0,     0,     0,     0,     0,     0,    27,    28,
      29,    30,    31,    32,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      33,    34,    35,    57,     0,     8,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    36,     0,     0,    37,
      38,     0,     0,     0,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,     0,     0,     0,    39,     0,     0,    40,    41,
      27,    28,    29,    30,    31,    32,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,   196,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,    37,    38,     0,     0,     0,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,     0,     0,     0,    39,     0,     0,
      40,    41,     0,    28,    29,    30,    31,    32,     0,     0,
       0,     0,     0,     0,     0,     0,   481,     0,     0,     0,
       0,     0,     0,     0,    33,    34,    35,     0,     8,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,    37,    38,     0,     0,     9,    10,    11,
      12,    13,    14,    15,    16,   903,    18,   904,    20,   905,
     906,    23,    24,    25,    26,   441,   442,   443,   444,    39,
     445,     0,     0,    41,    28,    29,    30,    31,    32,     0,
       0,     0,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,    33,    34,    35,     0,     0,
       8,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,   244,     0,    37,    38,     0,     0,     0,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,     0,     0,     0,
      39,     0,     0,     0,    41,   907,    28,    29,    30,    31,
      32,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
       0,     0,     0,     0,     0,     8,     0,     0,     0,     0,
       0,     0,     0,    36,   936,   363,    37,    38,     0,     0,
       0,     0,     0,     0,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    39,     0,     0,     0,    41,   907,     0,     0,
      27,    28,    29,    30,    31,    32,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    33,    34,    35,     0,     0,     8,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    36,     0,
       0,    37,    38,     0,     0,     0,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,     0,     0,     0,    39,     0,     0,
      40,    41,    27,    28,    29,    30,    31,    32,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    33,    34,    35,     0,     8,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      36,     0,     0,    37,    38,     0,     0,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,   226,     0,     0,     0,    39,
       0,     0,    40,    41,    28,    29,    30,    31,    32,     0,
       0,     0,     0,     0,     0,     0,     0,   622,     0,     0,
       0,     0,     0,     0,     0,    33,    34,    35,     0,     8,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    36,     0,     0,    37,    38,     0,     0,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,   441,   442,   443,   444,
      39,   445,     0,     0,    41,    28,    29,    30,    31,    32,
       0,     0,     0,     0,   446,   447,   448,   449,   450,   451,
     452,   453,   454,   455,   456,   457,    33,    34,    35,     0,
       8,     0,     0,     0,   624,     0,     0,     0,     0,     0,
       0,     0,    36,   244,     0,    37,    38,     0,     0,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,     0,     0,     0,
       0,    39,     0,     0,     0,    41,    28,    29,    30,    31,
      32,     0,     0,   441,   442,   443,   444,     0,   445,     0,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
       8,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,    36,     0,     0,    37,    38,     0,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,     0,     0,     0,
       0,     0,    39,     0,     0,     0,    41,     0,     0,    31,
      32,     0,     0,     0,     0,     0,     0,     0,     0,   741,
       0,     0,     0,     0,     0,     0,     0,    33,    34,    35,
       8,   747,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    36,     0,     0,    37,    38,     0,     9,
      10,    11,    12,    13,    14,    15,    16,   694,    18,   695,
      20,   696,   697,    23,    24,    25,    26,     0,   441,   442,
     443,   444,    39,   445,     0,     0,    41,     0,     0,     0,
     441,   442,   443,   444,     0,   445,   446,   447,   448,   449,
     450,   451,   452,   453,   454,   455,   456,   457,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
       0,     0,     0,    36,     0,     0,    37,    38,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    39,     0,     0,     0,    41
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-957))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       0,   581,    21,   157,   235,     5,    60,    71,    93,     1,
       2,     5,   149,   544,     1,     2,     5,     5,   356,   138,
     370,    21,   525,    40,   725,   140,   698,     5,   663,    93,
      94,   556,     5,    44,   516,   103,   848,     5,   220,    46,
      41,   178,   179,   525,     5,   420,    46,   378,   155,     5,
      46,    99,     6,     6,    54,     6,   238,   944,     6,   946,
      41,     6,   286,     3,    46,    42,     0,     4,    60,     6,
      70,   188,     5,    48,   138,    75,    76,   194,  1034,   118,
      45,     5,    47,   200,    41,    54,     5,     5,   205,     6,
     138,     5,   793,     5,     5,     5,    43,   658,    46,    46,
       3,    46,   150,    54,    40,   666,    43,   290,   291,    46,
     110,   294,   129,   337,   231,   111,    42,     5,     4,   117,
       6,   228,   140,     3,   241,   126,  1082,   104,   129,    76,
     691,    85,    85,   664,     5,   116,     5,    85,     3,     5,
     140,     5,   140,   191,   956,     5,   707,   116,   140,    89,
      90,  1038,   206,   140,     3,   209,   161,   162,   197,   116,
     167,     5,   169,   155,   514,   118,   171,   167,    85,   169,
      48,   872,   226,   155,    48,   115,     3,   114,   104,   119,
     234,   163,   289,   533,    43,   185,    89,    90,   153,   228,
     126,   863,   893,   129,   569,   117,   136,     1,   163,   199,
     136,     3,   356,   310,   535,   244,     6,   246,    54,    89,
      90,    76,   115,   103,   206,   215,   119,   209,   140,   402,
     403,   404,    40,     3,    89,    90,     6,  1039,   228,    40,
     230,   231,    52,    53,   226,   235,    40,  1049,    40,   119,
      89,    90,   234,   235,   103,   245,   228,   247,   235,    54,
     115,    71,    72,   814,   119,    57,   238,   103,     3,    59,
      40,     4,    89,    90,     3,    43,   273,     3,    48,   323,
     971,   271,   326,   273,     4,     3,   807,   802,    89,    90,
      43,   916,     3,   103,    75,   290,   291,    89,    90,   294,
      43,   273,   119,     6,   477,   478,    44,   289,   103,   384,
       4,    40,     6,    46,    40,   120,   809,   289,    43,    89,
      90,    40,    48,   115,   378,   546,    46,   119,   310,   110,
     384,    57,   437,   698,   329,   330,   998,   809,   310,    77,
      43,   323,   707,   515,   326,   115,   138,    40,   320,   119,
     690,    76,    46,  1015,    89,    90,   126,   339,   355,   129,
      89,    90,   339,    89,    90,   355,   136,   918,   138,    46,
     542,    89,    90,   427,    92,   419,    54,   550,    89,    90,
     370,  1072,   354,    42,   119,    43,   115,    40,     3,   115,
     119,  1064,    52,   119,   964,    54,    54,   115,    76,   920,
     126,   119,    43,   129,   115,    46,   396,     3,   119,   138,
     136,    71,   138,    40,     4,   405,     6,     4,   525,     6,
       3,    48,    40,    41,  1097,   463,   416,   417,   105,   106,
     420,    40,   983,    40,   424,    76,   408,   419,   482,    57,
     805,   992,    43,   487,    40,    46,    42,   419,    57,    40,
      57,    40,    48,    43,   426,   437,    46,     3,    54,   103,
     437,    57,   103,    46,    54,   420,    52,    53,    57,   464,
     465,   466,   116,   117,    89,    90,    84,    85,   473,   474,
      76,   535,    52,    53,     3,    71,    72,     6,   483,    43,
       3,    40,    46,    89,    90,   524,   140,    46,   863,   528,
     482,    71,    72,  1054,   119,   487,    89,    90,   104,   504,
    1061,    49,    50,    51,    40,    41,    40,   103,    40,   115,
      40,   511,   551,   119,   514,    40,    46,    40,    43,   857,
     126,    57,   115,   129,    40,    48,   119,    40,    41,    40,
     136,   531,   138,   533,   534,   527,    40,   531,    40,   539,
     527,    57,   531,   531,    57,   534,    40,   558,    40,    41,
      48,   543,    42,   531,   546,    40,   543,    47,   531,   546,
      89,    90,   569,   531,   571,    57,    89,    90,     6,   569,
     531,   571,   572,    40,   581,   531,   105,   106,   760,   761,
      48,   581,    48,   583,    40,    41,    43,    40,   136,    46,
      43,    41,   115,    46,    40,    41,   119,   579,   531,   581,
      48,    57,   607,   126,   105,   106,   129,   531,   672,  1076,
      54,    57,   531,   531,   579,   138,   116,   531,    42,   531,
     531,   531,   676,   998,    43,   664,  1093,  1094,  1095,    46,
       6,    43,    76,   770,    46,    43,    80,    46,    46,  1106,
    1015,   641,    54,   531,    88,    89,    90,   641,    43,   649,
      43,    46,   641,   641,   708,    41,    42,     3,   658,    54,
     531,   927,   531,   641,   930,   531,   666,   531,   641,    42,
     670,   531,   104,   641,   713,   116,   670,   696,    41,    42,
     641,   670,   670,   802,   676,   641,  1061,   531,   688,   689,
     690,   691,   670,   658,   811,    42,   696,   670,   698,   688,
     688,   666,   670,   857,   935,    40,   706,   707,   641,   670,
     688,   718,   706,   828,   670,   688,   708,   641,   718,     7,
     731,    48,   641,   641,    48,   725,   691,   641,    41,   641,
     641,   641,   739,   698,    41,    42,   821,   670,   802,   739,
     104,    41,   707,    89,    90,    41,   670,   116,   730,   731,
     804,   670,   670,   641,   802,    48,   670,   821,   670,   670,
     670,   898,    57,  1101,    40,   730,     3,    48,    48,   115,
     641,   810,   641,   119,   136,   641,    48,   641,   842,   818,
     126,   641,   670,   129,   116,   833,    43,    42,    43,    41,
     136,    46,    41,   793,   842,   849,   950,   641,    41,   670,
      46,   670,   856,    40,   670,    41,   670,   121,   122,   123,
     670,    48,   804,    80,   814,   879,    54,    54,    42,   819,
      57,    88,    89,    90,    41,    41,   670,    41,   828,   130,
     131,   132,   133,   134,   135,   136,   828,    43,    41,    76,
      41,   828,    41,   907,   844,    41,    21,    41,   104,   814,
     844,    42,    89,    90,     3,   844,   844,   849,   116,   859,
      49,    50,    51,   863,   856,   865,   844,    41,   116,    44,
     909,   844,   872,   134,   135,   136,   844,    43,   115,   111,
     919,    76,   119,   844,    46,    48,   905,   116,   844,   126,
     116,    40,   129,   893,    73,    74,    75,   936,   863,   136,
      48,   138,    77,    78,    79,   905,    48,    82,   947,    84,
      85,   844,   132,   133,   134,   135,   136,    41,   918,   973,
     844,   960,    41,   977,    46,   844,   844,    48,    48,    43,
     844,    48,   844,   844,   844,   935,    48,  1091,    48,    43,
      89,    90,    41,   935,   944,    43,   946,  1101,   935,    40,
     944,    47,   946,   918,    43,    91,   844,   964,   958,   959,
      57,    42,    41,    40,   964,    46,   115,  1021,    46,  1023,
     119,   971,    53,   844,    90,   844,    89,  1016,   844,    48,
     844,   973,   964,   983,   844,   977,    78,    41,    41,   138,
      48,     3,   992,   975,    42,   977,   107,   108,   998,  1063,
     844,    48,  1056,  1057,  1058,    86,    87,    48,    48,    48,
      48,    48,    42,    40,   140,  1015,   103,    62,   983,   130,
     131,   132,   133,   134,   135,   136,  1080,   992,    40,  1021,
      54,  1023,     3,   998,    41,     6,    48,   118,  1038,    47,
      42,    41,    41,    41,  1038,    40,    40,    48,  1048,    54,
    1015,    48,    71,    76,  1054,    48,    48,    48,    46,    76,
      76,  1061,   138,    47,  1056,  1057,  1058,   148,    87,    47,
      41,    43,  1072,  1073,   155,    41,  1076,    89,    90,    41,
     161,   162,    21,   164,    43,    54,    76,    41,  1080,  1054,
     171,    47,    40,  1093,  1094,  1095,  1061,    48,    48,    69,
      76,    76,    76,   115,    41,    44,  1106,   119,  1073,    76,
      42,  1076,    76,    43,   126,    43,   197,   129,    89,    90,
     201,   104,    92,    41,   136,    54,   138,    97,  1093,  1094,
    1095,   212,    41,    43,   105,   106,    48,    40,    77,    78,
      79,  1106,    47,    82,    83,    84,    85,   228,    43,    49,
      50,    51,    76,    43,     3,   236,    43,   238,   239,   240,
       3,   242,   104,   244,   245,   246,   247,    43,    43,   188,
      42,    48,    41,    47,   193,   194,    41,    48,    40,   118,
      80,   200,    40,   202,   203,    41,   205,    48,    88,    89,
      90,    40,   273,    43,    43,    41,    48,    40,   217,    48,
      41,   220,   221,    40,    48,    41,    48,    41,   289,   290,
     291,    41,   231,   294,    57,    46,    48,    41,   188,   238,
      48,    43,   241,   193,   194,    43,   245,   516,   539,   310,
     200,   312,  1016,   688,   935,   205,   546,   689,   511,   688,
      89,    90,   828,   688,   583,   688,    89,    90,   329,   330,
     220,   437,   556,   334,    51,     3,     4,     3,     6,   996,
       3,   231,   232,  1048,   424,   346,   115,   237,   238,   959,
     119,   241,   115,   354,   865,  1061,   119,   126,   564,  1043,
     129,   673,    -1,   126,    -1,    -1,   129,   136,    -1,   138,
     371,    -1,    40,    -1,    40,    43,    -1,    40,    46,    -1,
      48,    -1,    48,    -1,    -1,    48,    -1,    -1,    -1,   162,
      -1,    57,    -1,    -1,    57,    -1,    -1,     3,   171,    -1,
      -1,   402,   403,   404,    -1,    -1,    -1,   408,    -1,   410,
     411,    -1,   413,    -1,    -1,    -1,    -1,    -1,   419,    -1,
      -1,    89,    90,    89,    90,    -1,    89,    90,    -1,    -1,
     158,   159,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,   114,   115,    -1,   115,
      -1,   119,   115,   119,    -1,     3,   119,    -1,   126,    -1,
     126,   129,    -1,   129,    -1,   466,    -1,   240,   136,   242,
     138,    -1,   138,     3,    -1,   138,   477,   478,    -1,    -1,
      40,    -1,   483,    89,    90,    -1,    -1,    -1,    -1,    49,
      50,    51,    40,    -1,    54,    43,    -1,    -1,    -1,    -1,
      48,    -1,    -1,   504,    -1,    -1,   507,    -1,    -1,   115,
      40,    -1,    -1,   119,    -1,    -1,    -1,    -1,    48,    -1,
      80,    -1,    -1,   524,    -1,    -1,    -1,   528,    88,    89,
      90,   259,   138,   261,   262,   263,   264,   265,   266,   267,
      -1,    89,    90,    -1,     3,    -1,    -1,     6,    -1,   550,
     551,   552,    -1,    -1,   555,   556,   329,   330,    -1,    89,
      90,   334,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,
      -1,   119,     3,   301,    -1,    -1,   515,    -1,   126,    -1,
     581,   129,    -1,    -1,    -1,   115,   525,    -1,   136,   119,
     138,    -1,    -1,    -1,    -1,    -1,   324,    -1,    -1,   327,
     601,   540,    -1,   542,    -1,    -1,    -1,    -1,   138,    40,
      -1,    -1,    43,    -1,   615,   616,    -1,    48,    -1,   620,
      -1,    -1,   512,    -1,   625,   515,   516,    -1,    -1,   630,
      89,    90,    -1,   523,   524,   525,    -1,   410,   411,    -1,
     413,    -1,    -1,    -1,    -1,    -1,   105,   106,    -1,    -1,
      -1,    -1,   542,    -1,   544,    -1,   115,    -1,    89,    90,
     119,   551,    -1,   664,    -1,    -1,    -1,   126,    -1,    -1,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,    -1,    -1,   115,    -1,   687,    -1,   119,    -1,
      -1,    -1,   693,    -1,    -1,   126,    -1,    -1,   129,    -1,
       3,    -1,    -1,    -1,    -1,   136,    -1,   138,    -1,    -1,
     483,    -1,   713,   441,   442,   443,   444,   445,   446,   447,
     448,   449,   450,   451,   452,   453,   454,   455,   456,   457,
      -1,   504,    -1,   672,   507,    -1,    -1,    40,    -1,    -1,
      43,    -1,    -1,     3,    -1,    48,    -1,    -1,    -1,   750,
     751,   479,    -1,    -1,    -1,   756,    -1,    -1,    -1,    -1,
     488,    -1,    -1,    49,    50,    51,    -1,    -1,    54,    -1,
      -1,    -1,    -1,    -1,   664,    -1,    -1,    -1,     3,   552,
      40,   671,   555,   673,    -1,    -1,    89,    90,    48,    -1,
      76,    -1,    -1,    -1,    80,    -1,    -1,    -1,    -1,    -1,
      -1,   802,    88,    89,    90,    -1,    -1,    -1,    -1,   810,
      -1,    -1,   115,    -1,    -1,    40,   119,   818,    -1,    -1,
      -1,   760,   761,   126,    -1,    -1,   129,    -1,    -1,    89,
      90,    -1,    57,   136,    -1,   138,    -1,    -1,    -1,    -1,
      -1,    -1,   615,   616,    -1,    -1,    -1,   620,   576,   577,
     578,   579,   625,    -1,    -1,   115,    -1,   630,    -1,   119,
      -1,    -1,    -1,    -1,    89,    90,   126,    -1,   869,   129,
     760,   761,   811,    -1,    -1,    -1,   136,    -1,   138,    -1,
      -1,    -1,    -1,    -1,   885,   824,   825,   826,   889,   890,
     115,    -1,    -1,     3,   119,    -1,    -1,    -1,    -1,    -1,
      -1,   126,    -1,    -1,   129,    -1,     3,    -1,   909,    -1,
      -1,     3,    -1,   138,   853,    -1,    -1,   807,   919,   809,
     693,   811,    -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,
      40,    -1,    -1,    -1,    -1,   936,    -1,    -1,    48,    -1,
      -1,    -1,   832,    40,    -1,    -1,   947,    -1,    40,     3,
     951,    48,    -1,    -1,    -1,    -1,    48,    -1,    -1,   960,
      57,    -1,     3,   964,    40,   966,   967,    -1,    -1,    -1,
      -1,    -1,    48,    -1,    -1,    -1,   977,   750,   751,    89,
      90,   709,    -1,   756,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    89,    90,    -1,   934,    -1,    89,    90,    40,
     728,   729,   730,    57,   732,   115,    -1,    48,    -1,   119,
     738,  1012,    -1,    89,    90,  1016,   126,    -1,   115,   129,
      -1,    -1,   119,   115,    -1,    -1,   136,   119,   138,   126,
     920,    -1,   129,    -1,   126,    89,    90,   129,    -1,   115,
      -1,   138,  1043,   119,   136,    -1,   138,    -1,    89,    90,
     126,    -1,    -1,   129,    -1,    -1,    -1,   996,    -1,    -1,
      -1,   115,   138,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    -1,   126,    -1,   115,   129,    -1,    -1,   119,    -1,
      -1,    -1,    -1,    -1,   138,   126,    -1,    -1,   129,     3,
      -1,    -1,     6,    -1,    -1,    -1,    -1,   138,    -1,    13,
      14,    15,    16,    17,    18,    19,    20,    -1,    -1,    -1,
      -1,     6,   885,    -1,    -1,    -1,   889,   890,    13,    14,
      15,    16,    17,    18,    19,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     1,    -1,     3,     4,    -1,
      -1,    -1,     8,     9,    10,    -1,    -1,    -1,   876,   877,
      -1,    -1,    -1,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    89,    90,    43,    44,    -1,
      -1,    47,    -1,    49,    50,    51,    52,    53,    -1,    -1,
      -1,   105,   106,   966,   967,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      -1,    77,    78,    79,    -1,    81,    82,    83,    84,    85,
      86,    87,    -1,    89,    90,    91,    -1,    -1,    -1,    95,
      -1,    97,    98,    99,   100,   101,   102,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   113,   976,   115,
      -1,    -1,   118,   119,   120,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,    -1,   995,     6,     7,
      41,    -1,   138,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    55,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    -1,   107,   108,   109,   110,
      -1,   112,    -1,   107,   108,   109,   110,     3,    86,    -1,
      -1,    89,    90,    -1,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   130,   131,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,   115,    -1,    -1,
     118,   119,    -1,    -1,    40,    -1,    -1,    -1,   126,    -1,
      -1,   129,    48,     3,   132,   133,     6,     7,   136,   137,
     138,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    -1,    89,    90,    76,    -1,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    55,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
      70,    71,    72,   119,    -1,    -1,   107,   108,   109,   110,
      -1,   112,    -1,    -1,    -1,     3,    86,    -1,    -1,    89,
      90,    -1,   138,    -1,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,
      -1,    -1,    40,    -1,    -1,    -1,   126,    -1,    -1,   129,
      48,     3,   132,   133,     6,     7,   136,   137,   138,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    -1,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    89,    90,    -1,    -1,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    55,    -1,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,    70,    71,
      72,   119,    -1,   107,   108,   109,   110,    -1,   112,    -1,
      -1,    -1,    -1,     3,    86,    -1,    -1,    89,    90,    -1,
     138,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,    -1,   119,    -1,    -1,
      40,    -1,    -1,    -1,   126,    -1,    -1,   129,    48,     3,
     132,   133,     6,     7,   136,   137,   138,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    -1,    89,
      90,    -1,    -1,    -1,    -1,    49,    50,    51,    52,    53,
      -1,    55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   115,    70,    71,    72,   119,
      -1,   107,   108,   109,   110,    -1,    -1,    -1,    -1,    -1,
      -1,     3,    86,    -1,    -1,    89,    90,    -1,   138,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   115,    -1,    -1,    -1,   119,    -1,    -1,    40,    -1,
      -1,    -1,   126,    -1,    -1,   129,    48,     3,   132,   133,
       6,     7,   136,   137,   138,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    -1,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    89,    90,    -1,
      -1,    -1,    -1,    49,    50,    51,    52,    53,    -1,    55,
      -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    70,    71,    72,   119,   107,   108,
     109,   110,    -1,    -1,    -1,    -1,   107,   108,   109,   110,
      86,    -1,    -1,    89,    90,    -1,   138,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   129,   130,
     131,   132,   133,   134,   135,   136,    -1,    -1,    -1,   115,
      -1,    -1,    41,   119,    -1,    -1,    -1,    -1,    -1,    -1,
     126,    -1,    -1,   129,    -1,     3,   132,   133,     6,     7,
     136,   137,   138,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    49,    50,    51,    52,    53,    -1,    55,   107,   108,
     109,   110,    -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    -1,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,    86,    -1,
      -1,    89,    90,    22,    23,    24,    25,    26,    27,    28,
      29,    -1,    31,    -1,    33,    -1,    -1,    36,    37,    38,
      39,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,
      -1,   119,    43,    -1,    -1,    -1,    -1,    -1,   126,    -1,
      -1,   129,    -1,     3,   132,   133,     6,     7,   136,   137,
     138,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    55,   107,   108,   109,   110,
      -1,   112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    72,    -1,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,    86,    -1,    -1,    89,
      90,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   115,    -1,    -1,    -1,   119,
      -1,    -1,    -1,    -1,    -1,    -1,   126,    -1,    -1,   129,
      -1,     3,   132,   133,     6,     7,   136,   137,   138,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    -1,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    55,   107,   108,   109,   110,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,
      72,    -1,    -1,    -1,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,    86,    -1,    -1,    89,    90,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   115,    -1,    -1,    -1,   119,    -1,    -1,
      -1,    -1,    -1,    -1,   126,    -1,    -1,   129,    -1,     3,
     132,   133,     6,     7,   136,   137,   138,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,     3,    -1,
      -1,    -1,    -1,    -1,     3,    49,    50,    51,    52,    53,
       3,    55,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    40,    86,    48,    -1,    89,    90,    40,    -1,    48,
      -1,    -1,    57,    -1,    -1,    48,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,   115,    -1,    -1,    -1,   119,    -1,   107,   108,   109,
     110,    -1,    -1,    -1,    89,    90,    -1,    -1,   132,   133,
      89,    90,   136,   137,   138,    -1,    89,    90,   128,   129,
     130,   131,   132,   133,   134,   135,   136,    -1,    -1,    -1,
     115,    -1,    -1,    -1,   119,    -1,   115,    -1,    -1,    -1,
     119,    -1,   115,    -1,    -1,    -1,   119,     3,    -1,    -1,
      -1,    -1,    -1,   138,    -1,    -1,    -1,    -1,    -1,   138,
      -1,    -1,    -1,    -1,    -1,   138,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    89,    90,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,
      -1,    -1,   118,   119,     3,    -1,    -1,    -1,    -1,    -1,
     126,    -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,
     136,    -1,    -1,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,
      49,    50,    51,    52,    53,    -1,    -1,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    70,    71,    72,    -1,    -1,     3,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,
      89,    90,    -1,    -1,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,   115,    -1,    -1,   118,
     119,    48,    49,    50,    51,    52,    53,   126,    -1,    -1,
     129,    -1,    -1,    -1,    -1,    -1,    -1,   136,    -1,    -1,
      -1,    -1,    -1,    70,    71,    72,     3,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    89,    90,    -1,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    -1,    -1,    -1,    -1,    -1,   115,    -1,
      -1,   118,   119,    -1,    -1,    52,    53,    -1,    -1,   126,
      -1,    -1,   129,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    -1,    -1,    70,    71,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      -1,    -1,    89,    90,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   115,    -1,
       0,     1,   119,     3,     4,    -1,    -1,    -1,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   136,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,    -1,    -1,    43,    44,    -1,    -1,    -1,    -1,    49,
      50,    51,    52,    53,    -1,    -1,    56,    -1,    -1,    -1,
      -1,    -1,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    -1,    -1,    -1,    -1,    77,    78,    79,
      -1,    81,    82,    -1,    84,    85,    86,    87,    -1,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,     1,    -1,     3,     4,    -1,    -1,    -1,
       8,     9,    10,   113,    -1,   115,    -1,    -1,   118,   119,
      -1,    -1,    -1,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    -1,    -1,    -1,    43,    44,    -1,    -1,    47,
      -1,    49,    50,    51,    52,    53,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    -1,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,    77,
      78,    79,    -1,    81,    82,    -1,    84,    85,    86,    87,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,     1,    -1,     3,     4,    -1,
      -1,    -1,     8,     9,    10,   113,    -1,   115,    -1,    -1,
     118,   119,    -1,    -1,    -1,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,    43,    44,    -1,
      -1,    47,    -1,    49,    50,    51,    52,    53,    -1,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    77,    78,    79,    -1,    81,    82,    -1,    84,    85,
      86,    87,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,     1,    -1,     3,
       4,    -1,    -1,    -1,     8,     9,    10,   113,    -1,   115,
      -1,    -1,   118,   119,    -1,    -1,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    -1,    -1,    -1,    43,
      44,    -1,    -1,    47,    -1,    49,    50,    51,    52,    53,
      -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    -1,
      -1,    -1,    -1,    77,    78,    79,    -1,    81,    82,    -1,
      84,    85,    86,    87,    -1,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,     1,
      -1,     3,     4,    -1,    -1,    -1,     8,     9,    10,   113,
      -1,   115,    -1,    -1,   118,   119,    -1,    -1,    -1,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    -1,    -1,
      -1,    43,    44,    -1,    -1,    -1,    -1,    49,    50,    51,
      52,    53,    -1,    -1,    56,    -1,    -1,    -1,    60,    -1,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    77,    78,    79,    -1,    81,
      82,    -1,    84,    85,    86,    87,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,     1,    -1,     3,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   113,    -1,   115,    -1,    -1,   118,   119,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    48,    49,
      50,    51,    52,    53,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      70,    71,    72,     1,    -1,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,    -1,    89,
      90,    -1,    -1,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    -1,    -1,    -1,   115,    -1,    -1,   118,   119,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,     1,    -1,     3,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    89,    90,    -1,    -1,    -1,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,   115,    -1,    -1,
     118,   119,    -1,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    -1,     3,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    89,    90,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,   107,   108,   109,   110,   115,
     112,    -1,    -1,   119,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,    70,    71,    72,    -1,    -1,
       3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    -1,    89,    90,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
     115,    -1,    -1,    -1,   119,   120,    49,    50,    51,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,
      -1,    -1,    -1,    -1,    -1,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    13,    89,    90,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,   115,    -1,    -1,    -1,   119,   120,    -1,    -1,
      48,    49,    50,    51,    52,    53,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    70,    71,    72,    -1,    -1,     3,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    -1,
      -1,    89,    90,    -1,    -1,    -1,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    -1,    -1,    -1,   115,    -1,    -1,
     118,   119,    48,    49,    50,    51,    52,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    70,    71,    72,    -1,     3,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    -1,    -1,    89,    90,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    -1,    -1,    -1,   115,
      -1,    -1,   118,   119,    49,    50,    51,    52,    53,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    70,    71,    72,    -1,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    89,    90,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,   107,   108,   109,   110,
     115,   112,    -1,    -1,   119,    49,    50,    51,    52,    53,
      -1,    -1,    -1,    -1,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,    70,    71,    72,    -1,
       3,    -1,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    -1,    89,    90,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,   115,    -1,    -1,    -1,   119,    49,    50,    51,    52,
      53,    -1,    -1,   107,   108,   109,   110,    -1,   112,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,
       3,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,    86,    -1,    -1,    89,    90,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,    -1,   119,    -1,    -1,    52,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    71,    72,
       3,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    -1,    -1,    89,    90,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    -1,   107,   108,
     109,   110,   115,   112,    -1,    -1,   119,    -1,    -1,    -1,
     107,   108,   109,   110,    -1,   112,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
      -1,    -1,    -1,    86,    -1,    -1,    89,    90,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   115,    -1,    -1,    -1,   119
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   121,   122,   123,   142,   143,   307,     1,     3,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    48,    49,    50,
      51,    52,    53,    70,    71,    72,    86,    89,    90,   115,
     118,   119,   193,   235,   249,   250,   252,   253,   254,   255,
     256,   257,   282,   283,   293,   296,   298,     1,   235,     1,
      40,     0,     1,     4,     8,     9,    10,    21,    43,    44,
      56,    62,    63,    64,    65,    66,    67,    68,    69,    77,
      78,    79,    81,    82,    84,    85,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   113,   118,
     144,   145,   146,   148,   149,   150,   151,   152,   155,   156,
     158,   159,   160,   161,   162,   163,   164,   167,   168,   169,
     172,   174,   179,   180,   181,   182,   184,   188,   195,   196,
     197,   198,   199,   203,   204,   211,   212,   223,   231,   307,
     103,   292,   307,    48,    52,    71,    48,    48,    40,   140,
     103,   296,    43,   253,   249,    40,    48,    54,    57,    76,
     119,   126,   129,   136,   138,   240,   241,   243,   245,   246,
     247,   248,   296,   307,   249,   256,   296,   292,   117,   140,
     297,    43,    43,   232,   233,   235,   307,   120,    40,     6,
      85,   118,   301,    40,   304,   307,     1,   251,   252,   293,
      40,   304,    40,   166,   307,    40,    40,    84,    85,    40,
      44,    77,    92,   296,    46,   293,   296,    40,     4,    46,
      40,    40,    43,    46,     4,   301,    40,   178,   251,   176,
     178,    40,    40,   301,    40,   103,   283,   304,    40,   126,
     129,   243,   248,   296,    87,   193,   251,   283,     7,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    40,
      55,   126,   129,   132,   133,   136,   137,   138,   235,   236,
     237,   239,   251,   252,   268,   269,   270,   271,   301,   302,
     307,    48,    48,    48,   296,   119,   298,   283,   249,    40,
     126,   129,   232,   246,   248,   296,    48,    46,   105,   106,
     258,   259,   260,   261,   262,    58,   268,   269,   268,     3,
      40,    48,   138,   244,   247,   296,    48,   244,   247,   248,
     249,   296,   240,    40,    57,   240,    40,    57,    48,   126,
     129,   244,   247,   296,   116,   298,   119,   298,    41,    42,
     234,   307,   260,   293,   294,   301,   283,     6,    46,    85,
     294,   305,   294,    43,    40,   243,    54,    41,   294,   296,
     293,   293,   294,    13,   173,   232,   232,   296,    43,    54,
     214,    54,    46,   293,   175,   305,   293,   232,    46,   242,
     243,   246,   307,    43,    42,   177,   307,   294,   295,   307,
     153,   154,   301,   232,   207,   208,   209,   235,   282,   307,
     296,   301,   126,   129,   248,   293,   296,   305,    40,   294,
     126,   129,   296,   116,   243,   296,   263,   293,   307,    40,
     243,    76,   274,   275,   296,   307,   268,    40,    48,   268,
     268,   268,   268,   268,   268,   268,   104,    42,   238,   307,
      40,   107,   108,   109,   110,   112,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,     7,    48,
      48,    41,   297,   104,   126,   129,   248,   296,   245,   296,
     245,    41,    41,   126,   129,   245,   296,   116,    48,    57,
     268,    58,    40,   248,   296,    48,   296,    40,    57,    48,
     248,   232,    58,   268,   232,    58,   268,    48,    48,   244,
     247,    48,   244,   247,   116,    48,   126,   129,   244,   297,
      43,   235,    41,   296,   183,    42,    54,    41,   240,   258,
      41,    46,    41,    54,    41,    42,   171,    42,    41,    41,
      43,   143,   296,   213,    41,    41,    41,    41,   176,   178,
      41,    41,    42,    46,    41,   104,    42,   210,   307,    59,
     116,    41,   248,   296,    43,   116,   111,    54,    76,   194,
     307,   232,   296,    80,    88,    89,    90,   186,   240,   249,
     285,   286,   276,    46,    43,   274,    41,   126,   129,   136,
     248,   251,    48,   239,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   268,
     268,   283,   296,   116,    41,    41,    41,   116,   245,   245,
     268,   232,   244,   296,    41,   116,    48,   232,    58,   268,
      48,    41,    58,    41,    58,    48,    48,    48,    48,   126,
     129,   244,   247,    48,    48,    48,   244,   234,     4,    46,
     301,   143,   305,   153,   270,   301,   306,    43,    43,   147,
       4,   165,   301,     4,    43,    46,   114,   170,   243,   301,
     303,   294,   301,   306,    41,   235,   243,    46,    47,    43,
     143,    44,   231,   176,    43,    46,    40,    47,   177,   115,
     119,   293,   299,    43,   305,   235,   170,    91,   205,   209,
     157,   243,   301,   116,    30,    32,    34,    35,   187,   254,
     255,   296,    57,   189,   253,    43,    46,    41,    40,    40,
     285,    90,    89,    42,    43,    46,   185,   240,   286,   240,
      78,   277,   278,   284,   307,   201,    46,   268,    41,    41,
     136,   249,    41,   126,   129,   241,    48,   238,    76,   296,
      41,    58,    41,    41,   244,   244,    41,    58,   244,   244,
      48,    48,    48,    48,    48,   244,    48,    48,    48,    47,
      42,    42,     1,    43,    66,    73,    74,    75,    78,    83,
     138,   148,   149,   150,   151,   155,   156,   160,   162,   164,
     167,   169,   172,   174,   179,   180,   181,   182,   199,   203,
     204,   211,   215,   219,   220,   221,   222,   223,   224,   225,
     226,   229,   231,   307,    40,   249,   286,   287,   307,    54,
      41,    42,   171,   170,   243,   287,    47,   301,   251,   293,
      43,    54,   303,   232,   140,   117,   140,   300,   103,    41,
      47,   296,    44,   118,   184,   199,   203,   204,   206,   220,
     222,   224,   231,   210,   143,   287,    43,   292,   186,    40,
      46,   190,   150,   264,   265,   307,    40,    54,   286,   287,
     288,   232,   268,   243,   240,    42,    73,    74,    75,   279,
     281,   215,   200,   268,   268,   268,    41,    41,    41,    40,
     268,   240,    41,   244,   244,    48,    48,    48,   244,    48,
      48,   305,   305,   218,    46,    76,    76,    76,   138,    40,
     298,    47,   215,    30,    32,    34,    35,   120,   230,   251,
     255,   296,   232,   286,   170,   301,   306,    43,   243,    41,
     287,    43,   243,    43,   178,    41,   119,   293,   293,   119,
     293,   236,     4,    46,    54,   103,    87,    60,    43,   185,
     232,    40,    43,   191,   266,   293,    42,    47,   232,   258,
      54,    76,   289,   307,    41,    41,   186,   278,   296,   280,
      47,   215,   268,   268,   251,   244,    48,    48,   244,   244,
     215,   216,   298,    40,   292,   251,    76,    40,    43,    41,
     171,   287,    43,   243,   170,    43,    43,   300,   300,   104,
     293,   207,    41,   192,   264,    54,   264,    43,   243,    41,
      43,   260,   290,   291,   296,    43,    46,   185,    48,   272,
     273,   307,   284,    43,   202,   243,    47,   241,   244,   244,
     215,    40,   232,    40,   126,   129,   248,   268,   232,    43,
      43,   287,    43,   104,   287,    43,   267,   268,   266,   186,
      43,    46,    43,    42,    48,    40,    46,    48,   296,   186,
     202,    41,    47,   232,    41,   232,    40,    40,    40,   129,
      43,    41,    43,   111,   190,   264,   185,   291,    48,    48,
     273,   185,   217,    41,   227,   287,    41,   232,   232,   232,
      40,   288,   251,   191,    48,    48,   215,   228,   287,    43,
      46,    54,   228,    41,    41,    41,   232,   190,    48,    43,
      46,    54,   260,   228,   228,   228,    41,   191,    48,   258,
      43,   228,    43
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 1463 "parser.y"
    {
                   if (!classes) classes = NewHash();
		   Setattr((yyvsp[(1) - (1)].node),"classes",classes); 
		   Setattr((yyvsp[(1) - (1)].node),"name",ModuleName);
		   
		   if ((!module_node) && ModuleName) {
		     module_node = new_node("module");
		     Setattr(module_node,"name",ModuleName);
		   }
		   Setattr((yyvsp[(1) - (1)].node),"module",module_node);
	           top = (yyvsp[(1) - (1)].node);
               }
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 1475 "parser.y"
    {
                 top = Copy(Getattr((yyvsp[(2) - (3)].p),"type"));
		 Delete((yyvsp[(2) - (3)].p));
               }
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 1479 "parser.y"
    {
                 top = 0;
               }
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 1482 "parser.y"
    {
                 top = (yyvsp[(2) - (3)].p);
               }
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 1485 "parser.y"
    {
                 top = 0;
               }
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 1488 "parser.y"
    {
                 top = (yyvsp[(3) - (5)].pl);
               }
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 1491 "parser.y"
    {
                 top = 0;
               }
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 1496 "parser.y"
    {  
                   /* add declaration to end of linked list (the declaration isn't always a single declaration, sometimes it is a linked list itself) */
                   appendChild((yyvsp[(1) - (2)].node),(yyvsp[(2) - (2)].node));
                   (yyval.node) = (yyvsp[(1) - (2)].node);
               }
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 1501 "parser.y"
    {
                   (yyval.node) = new_node("top");
               }
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 1506 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 1507 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 1508 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 1509 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 1510 "parser.y"
    {
                  (yyval.node) = 0;
		  Swig_error(cparse_file, cparse_line,"Syntax error in input(1).\n");
		  exit(1);
               }
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 1516 "parser.y"
    { 
                  if ((yyval.node)) {
   		      add_symbols((yyval.node));
                  }
                  (yyval.node) = (yyvsp[(1) - (1)].node); 
	       }
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 1532 "parser.y"
    {
                  (yyval.node) = 0;
                  skip_decl();
               }
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 1542 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 1543 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 1544 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 1545 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 1546 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 1547 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 1548 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 1549 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 1550 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 1551 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 1552 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 1553 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 1554 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 1555 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 1556 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 1557 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 1558 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 1559 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 1560 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 1561 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 1562 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 1569 "parser.y"
    {
               Node *cls;
	       String *clsname;
	       extendmode = 1;
	       cplus_mode = CPLUS_PUBLIC;
	       if (!classes) classes = NewHash();
	       if (!classes_typedefs) classes_typedefs = NewHash();
	       clsname = make_class_name((yyvsp[(3) - (4)].str));
	       cls = Getattr(classes,clsname);
	       if (!cls) {
	         cls = Getattr(classes_typedefs, clsname);
		 if (!cls) {
		   /* No previous definition. Create a new scope */
		   Node *am = Getattr(Swig_extend_hash(),clsname);
		   if (!am) {
		     Swig_symbol_newscope();
		     Swig_symbol_setscopename((yyvsp[(3) - (4)].str));
		     prev_symtab = 0;
		   } else {
		     prev_symtab = Swig_symbol_setscope(Getattr(am,"symtab"));
		   }
		   current_class = 0;
		 } else {
		   /* Previous typedef class definition.  Use its symbol table.
		      Deprecated, just the real name should be used. 
		      Note that %extend before the class typedef never worked, only %extend after the class typdef. */
		   prev_symtab = Swig_symbol_setscope(Getattr(cls, "symtab"));
		   current_class = cls;
		   SWIG_WARN_NODE_BEGIN(cls);
		   Swig_warning(WARN_PARSE_EXTEND_NAME, cparse_file, cparse_line, "Deprecated %%extend name used - the %s name '%s' should be used instead of the typedef name '%s'.\n", Getattr(cls, "kind"), SwigType_namestr(Getattr(cls, "name")), (yyvsp[(3) - (4)].str));
		   SWIG_WARN_NODE_END(cls);
		 }
	       } else {
		 /* Previous class definition.  Use its symbol table */
		 prev_symtab = Swig_symbol_setscope(Getattr(cls,"symtab"));
		 current_class = cls;
	       }
	       Classprefix = NewString((yyvsp[(3) - (4)].str));
	       Namespaceprefix= Swig_symbol_qualifiedscopename(0);
	       Delete(clsname);
	     }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 1609 "parser.y"
    {
               String *clsname;
	       extendmode = 0;
               (yyval.node) = new_node("extend");
	       Setattr((yyval.node),"symtab",Swig_symbol_popscope());
	       if (prev_symtab) {
		 Swig_symbol_setscope(prev_symtab);
	       }
	       Namespaceprefix = Swig_symbol_qualifiedscopename(0);
               clsname = make_class_name((yyvsp[(3) - (7)].str));
	       Setattr((yyval.node),"name",clsname);

	       mark_nodes_as_extend((yyvsp[(6) - (7)].node));
	       if (current_class) {
		 /* We add the extension to the previously defined class */
		 appendChild((yyval.node),(yyvsp[(6) - (7)].node));
		 appendChild(current_class,(yyval.node));
	       } else {
		 /* We store the extensions in the extensions hash */
		 Node *am = Getattr(Swig_extend_hash(),clsname);
		 if (am) {
		   /* Append the members to the previous extend methods */
		   appendChild(am,(yyvsp[(6) - (7)].node));
		 } else {
		   appendChild((yyval.node),(yyvsp[(6) - (7)].node));
		   Setattr(Swig_extend_hash(),clsname,(yyval.node));
		 }
	       }
	       current_class = 0;
	       Delete(Classprefix);
	       Delete(clsname);
	       Classprefix = 0;
	       prev_symtab = 0;
	       (yyval.node) = 0;

	     }
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 1651 "parser.y"
    {
                    (yyval.node) = new_node("apply");
                    Setattr((yyval.node),"pattern",Getattr((yyvsp[(2) - (5)].p),"pattern"));
		    appendChild((yyval.node),(yyvsp[(4) - (5)].p));
               }
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 1661 "parser.y"
    {
		 (yyval.node) = new_node("clear");
		 appendChild((yyval.node),(yyvsp[(2) - (3)].p));
               }
    break;

  case 43:

/* Line 1806 of yacc.c  */
#line 1672 "parser.y"
    {
		   if (((yyvsp[(4) - (5)].dtype).type != T_ERROR) && ((yyvsp[(4) - (5)].dtype).type != T_SYMBOL)) {
		     SwigType *type = NewSwigType((yyvsp[(4) - (5)].dtype).type);
		     (yyval.node) = new_node("constant");
		     Setattr((yyval.node),"name",(yyvsp[(2) - (5)].id));
		     Setattr((yyval.node),"type",type);
		     Setattr((yyval.node),"value",(yyvsp[(4) - (5)].dtype).val);
		     if ((yyvsp[(4) - (5)].dtype).rawval) Setattr((yyval.node),"rawval", (yyvsp[(4) - (5)].dtype).rawval);
		     Setattr((yyval.node),"storage","%constant");
		     SetFlag((yyval.node),"feature:immutable");
		     add_symbols((yyval.node));
		     Delete(type);
		   } else {
		     if ((yyvsp[(4) - (5)].dtype).type == T_ERROR) {
		       Swig_warning(WARN_PARSE_UNSUPPORTED_VALUE,cparse_file,cparse_line,"Unsupported constant value (ignored)\n");
		     }
		     (yyval.node) = 0;
		   }

	       }
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 1693 "parser.y"
    {
		 if (((yyvsp[(4) - (5)].dtype).type != T_ERROR) && ((yyvsp[(4) - (5)].dtype).type != T_SYMBOL)) {
		   SwigType_push((yyvsp[(2) - (5)].type),(yyvsp[(3) - (5)].decl).type);
		   /* Sneaky callback function trick */
		   if (SwigType_isfunction((yyvsp[(2) - (5)].type))) {
		     SwigType_add_pointer((yyvsp[(2) - (5)].type));
		   }
		   (yyval.node) = new_node("constant");
		   Setattr((yyval.node),"name",(yyvsp[(3) - (5)].decl).id);
		   Setattr((yyval.node),"type",(yyvsp[(2) - (5)].type));
		   Setattr((yyval.node),"value",(yyvsp[(4) - (5)].dtype).val);
		   if ((yyvsp[(4) - (5)].dtype).rawval) Setattr((yyval.node),"rawval", (yyvsp[(4) - (5)].dtype).rawval);
		   Setattr((yyval.node),"storage","%constant");
		   SetFlag((yyval.node),"feature:immutable");
		   add_symbols((yyval.node));
		 } else {
		     if ((yyvsp[(4) - (5)].dtype).type == T_ERROR) {
		       Swig_warning(WARN_PARSE_UNSUPPORTED_VALUE,cparse_file,cparse_line,"Unsupported constant value\n");
		     }
		   (yyval.node) = 0;
		 }
               }
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 1715 "parser.y"
    {
		 Swig_warning(WARN_PARSE_BAD_VALUE,cparse_file,cparse_line,"Bad constant value (ignored).\n");
		 (yyval.node) = 0;
	       }
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 1726 "parser.y"
    {
		 char temp[64];
		 Replace((yyvsp[(2) - (2)].str),"$file",cparse_file, DOH_REPLACE_ANY);
		 sprintf(temp,"%d", cparse_line);
		 Replace((yyvsp[(2) - (2)].str),"$line",temp,DOH_REPLACE_ANY);
		 Printf(stderr,"%s\n", (yyvsp[(2) - (2)].str));
		 Delete((yyvsp[(2) - (2)].str));
                 (yyval.node) = 0;
	       }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 1735 "parser.y"
    {
		 char temp[64];
		 String *s = NewString((yyvsp[(2) - (2)].id));
		 Replace(s,"$file",cparse_file, DOH_REPLACE_ANY);
		 sprintf(temp,"%d", cparse_line);
		 Replace(s,"$line",temp,DOH_REPLACE_ANY);
		 Printf(stderr,"%s\n", s);
		 Delete(s);
                 (yyval.node) = 0;
               }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 1754 "parser.y"
    {
                    skip_balanced('{','}');
		    (yyval.node) = 0;
		    Swig_warning(WARN_DEPRECATED_EXCEPT,cparse_file, cparse_line, "%%except is deprecated.  Use %%exception instead.\n");
	       }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 1760 "parser.y"
    {
                    skip_balanced('{','}');
		    (yyval.node) = 0;
		    Swig_warning(WARN_DEPRECATED_EXCEPT,cparse_file, cparse_line, "%%except is deprecated.  Use %%exception instead.\n");
               }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 1766 "parser.y"
    {
		 (yyval.node) = 0;
		 Swig_warning(WARN_DEPRECATED_EXCEPT,cparse_file, cparse_line, "%%except is deprecated.  Use %%exception instead.\n");
               }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 1771 "parser.y"
    {
		 (yyval.node) = 0;
		 Swig_warning(WARN_DEPRECATED_EXCEPT,cparse_file, cparse_line, "%%except is deprecated.  Use %%exception instead.\n");
	       }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 1778 "parser.y"
    {		 
                 (yyval.node) = NewHash();
                 Setattr((yyval.node),"value",(yyvsp[(1) - (4)].id));
		 Setattr((yyval.node),"type",Getattr((yyvsp[(3) - (4)].p),"type"));
               }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 1785 "parser.y"
    {
                 (yyval.node) = NewHash();
                 Setattr((yyval.node),"value",(yyvsp[(1) - (1)].id));
              }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 1789 "parser.y"
    {
                (yyval.node) = (yyvsp[(1) - (1)].node);
              }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 1802 "parser.y"
    {
                   Hash *p = (yyvsp[(5) - (7)].node);
		   (yyval.node) = new_node("fragment");
		   Setattr((yyval.node),"value",Getattr((yyvsp[(3) - (7)].node),"value"));
		   Setattr((yyval.node),"type",Getattr((yyvsp[(3) - (7)].node),"type"));
		   Setattr((yyval.node),"section",Getattr(p,"name"));
		   Setattr((yyval.node),"kwargs",nextSibling(p));
		   Setattr((yyval.node),"code",(yyvsp[(7) - (7)].str));
                 }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 1811 "parser.y"
    {
		   Hash *p = (yyvsp[(5) - (7)].node);
		   String *code;
                   skip_balanced('{','}');
		   (yyval.node) = new_node("fragment");
		   Setattr((yyval.node),"value",Getattr((yyvsp[(3) - (7)].node),"value"));
		   Setattr((yyval.node),"type",Getattr((yyvsp[(3) - (7)].node),"type"));
		   Setattr((yyval.node),"section",Getattr(p,"name"));
		   Setattr((yyval.node),"kwargs",nextSibling(p));
		   Delitem(scanner_ccode,0);
		   Delitem(scanner_ccode,DOH_END);
		   code = Copy(scanner_ccode);
		   Setattr((yyval.node),"code",code);
		   Delete(code);
                 }
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 1826 "parser.y"
    {
		   (yyval.node) = new_node("fragment");
		   Setattr((yyval.node),"value",Getattr((yyvsp[(3) - (5)].node),"value"));
		   Setattr((yyval.node),"type",Getattr((yyvsp[(3) - (5)].node),"type"));
		   Setattr((yyval.node),"emitonly","1");
		 }
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 1839 "parser.y"
    {
                     (yyvsp[(1) - (4)].loc).filename = Copy(cparse_file);
		     (yyvsp[(1) - (4)].loc).line = cparse_line;
		     scanner_set_location(NewString((yyvsp[(3) - (4)].id)),1);
                     if ((yyvsp[(2) - (4)].node)) { 
		       String *maininput = Getattr((yyvsp[(2) - (4)].node), "maininput");
		       if (maininput)
		         scanner_set_main_input_file(NewString(maininput));
		     }
               }
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 1848 "parser.y"
    {
                     String *mname = 0;
                     (yyval.node) = (yyvsp[(6) - (7)].node);
		     scanner_set_location((yyvsp[(1) - (7)].loc).filename,(yyvsp[(1) - (7)].loc).line+1);
		     if (strcmp((yyvsp[(1) - (7)].loc).type,"include") == 0) set_nodeType((yyval.node),"include");
		     if (strcmp((yyvsp[(1) - (7)].loc).type,"import") == 0) {
		       mname = (yyvsp[(2) - (7)].node) ? Getattr((yyvsp[(2) - (7)].node),"module") : 0;
		       set_nodeType((yyval.node),"import");
		       if (import_mode) --import_mode;
		     }
		     
		     Setattr((yyval.node),"name",(yyvsp[(3) - (7)].id));
		     /* Search for the module (if any) */
		     {
			 Node *n = firstChild((yyval.node));
			 while (n) {
			     if (Strcmp(nodeType(n),"module") == 0) {
			         if (mname) {
				   Setattr(n,"name", mname);
				   mname = 0;
				 }
				 Setattr((yyval.node),"module",Getattr(n,"name"));
				 break;
			     }
			     n = nextSibling(n);
			 }
			 if (mname) {
			   /* There is no module node in the import
			      node, ie, you imported a .h file
			      directly.  We are forced then to create
			      a new import node with a module node.
			   */			      
			   Node *nint = new_node("import");
			   Node *mnode = new_node("module");
			   Setattr(mnode,"name", mname);
			   appendChild(nint,mnode);
			   Delete(mnode);
			   appendChild(nint,firstChild((yyval.node)));
			   (yyval.node) = nint;
			   Setattr((yyval.node),"module",mname);
			 }
		     }
		     Setattr((yyval.node),"options",(yyvsp[(2) - (7)].node));
               }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 1894 "parser.y"
    { (yyval.loc).type = "include"; }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 1895 "parser.y"
    { (yyval.loc).type = "import"; ++import_mode;}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 1902 "parser.y"
    {
                 String *cpps;
		 if (Namespaceprefix) {
		   Swig_error(cparse_file, cparse_start_line, "%%inline directive inside a namespace is disallowed.\n");
		   (yyval.node) = 0;
		 } else {
		   (yyval.node) = new_node("insert");
		   Setattr((yyval.node),"code",(yyvsp[(2) - (2)].str));
		   /* Need to run through the preprocessor */
		   Seek((yyvsp[(2) - (2)].str),0,SEEK_SET);
		   Setline((yyvsp[(2) - (2)].str),cparse_start_line);
		   Setfile((yyvsp[(2) - (2)].str),cparse_file);
		   cpps = Preprocessor_parse((yyvsp[(2) - (2)].str));
		   start_inline(Char(cpps), cparse_start_line);
		   Delete((yyvsp[(2) - (2)].str));
		   Delete(cpps);
		 }
		 
	       }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 1921 "parser.y"
    {
                 String *cpps;
		 int start_line = cparse_line;
		 skip_balanced('{','}');
		 if (Namespaceprefix) {
		   Swig_error(cparse_file, cparse_start_line, "%%inline directive inside a namespace is disallowed.\n");
		   
		   (yyval.node) = 0;
		 } else {
		   String *code;
                   (yyval.node) = new_node("insert");
		   Delitem(scanner_ccode,0);
		   Delitem(scanner_ccode,DOH_END);
		   code = Copy(scanner_ccode);
		   Setattr((yyval.node),"code", code);
		   Delete(code);		   
		   cpps=Copy(scanner_ccode);
		   start_inline(Char(cpps), start_line);
		   Delete(cpps);
		 }
               }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 1952 "parser.y"
    {
                 (yyval.node) = new_node("insert");
		 Setattr((yyval.node),"code",(yyvsp[(1) - (1)].str));
	       }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 1956 "parser.y"
    {
		 String *code = NewStringEmpty();
		 (yyval.node) = new_node("insert");
		 Setattr((yyval.node),"section",(yyvsp[(3) - (5)].id));
		 Setattr((yyval.node),"code",code);
		 if (Swig_insert_file((yyvsp[(5) - (5)].id),code) < 0) {
		   Swig_error(cparse_file, cparse_line, "Couldn't find '%s'.\n", (yyvsp[(5) - (5)].id));
		   (yyval.node) = 0;
		 } 
               }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 1966 "parser.y"
    {
		 (yyval.node) = new_node("insert");
		 Setattr((yyval.node),"section",(yyvsp[(3) - (5)].id));
		 Setattr((yyval.node),"code",(yyvsp[(5) - (5)].str));
               }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 1971 "parser.y"
    {
		 String *code;
                 skip_balanced('{','}');
		 (yyval.node) = new_node("insert");
		 Setattr((yyval.node),"section",(yyvsp[(3) - (5)].id));
		 Delitem(scanner_ccode,0);
		 Delitem(scanner_ccode,DOH_END);
		 code = Copy(scanner_ccode);
		 Setattr((yyval.node),"code", code);
		 Delete(code);
	       }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 1989 "parser.y"
    {
                 (yyval.node) = new_node("module");
		 if ((yyvsp[(2) - (3)].node)) {
		   Setattr((yyval.node),"options",(yyvsp[(2) - (3)].node));
		   if (Getattr((yyvsp[(2) - (3)].node),"directors")) {
		     Wrapper_director_mode_set(1);
		     if (!cparse_cplusplus) {
		       Swig_error(cparse_file, cparse_line, "Directors are not supported for C code and require the -c++ option\n");
		     }
		   } 
		   if (Getattr((yyvsp[(2) - (3)].node),"dirprot")) {
		     Wrapper_director_protected_mode_set(1);
		   } 
		   if (Getattr((yyvsp[(2) - (3)].node),"allprotected")) {
		     Wrapper_all_protected_mode_set(1);
		   } 
		   if (Getattr((yyvsp[(2) - (3)].node),"templatereduce")) {
		     template_reduce = 1;
		   }
		   if (Getattr((yyvsp[(2) - (3)].node),"notemplatereduce")) {
		     template_reduce = 0;
		   }
		 }
		 if (!ModuleName) ModuleName = NewString((yyvsp[(3) - (3)].id));
		 if (!import_mode) {
		   /* first module included, we apply global
		      ModuleName, which can be modify by -module */
		   String *mname = Copy(ModuleName);
		   Setattr((yyval.node),"name",mname);
		   Delete(mname);
		 } else { 
		   /* import mode, we just pass the idstring */
		   Setattr((yyval.node),"name",(yyvsp[(3) - (3)].id));   
		 }		 
		 if (!module_node) module_node = (yyval.node);
	       }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 2032 "parser.y"
    {
                 Swig_warning(WARN_DEPRECATED_NAME,cparse_file,cparse_line, "%%name is deprecated.  Use %%rename instead.\n");
		 Delete(yyrename);
                 yyrename = NewString((yyvsp[(3) - (4)].id));
		 (yyval.node) = 0;
               }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 2038 "parser.y"
    {
		 Swig_warning(WARN_DEPRECATED_NAME,cparse_file,cparse_line, "%%name is deprecated.  Use %%rename instead.\n");
		 (yyval.node) = 0;
		 Swig_error(cparse_file,cparse_line,"Missing argument to %%name directive.\n");
	       }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 2051 "parser.y"
    {
                 (yyval.node) = new_node("native");
		 Setattr((yyval.node),"name",(yyvsp[(3) - (7)].id));
		 Setattr((yyval.node),"wrap:name",(yyvsp[(6) - (7)].id));
	         add_symbols((yyval.node));
	       }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 2057 "parser.y"
    {
		 if (!SwigType_isfunction((yyvsp[(7) - (8)].decl).type)) {
		   Swig_error(cparse_file,cparse_line,"%%native declaration '%s' is not a function.\n", (yyvsp[(7) - (8)].decl).id);
		   (yyval.node) = 0;
		 } else {
		     Delete(SwigType_pop_function((yyvsp[(7) - (8)].decl).type));
		     /* Need check for function here */
		     SwigType_push((yyvsp[(6) - (8)].type),(yyvsp[(7) - (8)].decl).type);
		     (yyval.node) = new_node("native");
	             Setattr((yyval.node),"name",(yyvsp[(3) - (8)].id));
		     Setattr((yyval.node),"wrap:name",(yyvsp[(7) - (8)].decl).id);
		     Setattr((yyval.node),"type",(yyvsp[(6) - (8)].type));
		     Setattr((yyval.node),"parms",(yyvsp[(7) - (8)].decl).parms);
		     Setattr((yyval.node),"decl",(yyvsp[(7) - (8)].decl).type);
		 }
	         add_symbols((yyval.node));
	       }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 2083 "parser.y"
    {
                 (yyval.node) = new_node("pragma");
		 Setattr((yyval.node),"lang",(yyvsp[(2) - (5)].id));
		 Setattr((yyval.node),"name",(yyvsp[(3) - (5)].id));
		 Setattr((yyval.node),"value",(yyvsp[(5) - (5)].str));
	       }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 2089 "parser.y"
    {
		(yyval.node) = new_node("pragma");
		Setattr((yyval.node),"lang",(yyvsp[(2) - (3)].id));
		Setattr((yyval.node),"name",(yyvsp[(3) - (3)].id));
	      }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 2096 "parser.y"
    { (yyval.str) = NewString((yyvsp[(1) - (1)].id)); }
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 2097 "parser.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 2100 "parser.y"
    { (yyval.id) = (yyvsp[(2) - (3)].id); }
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 2101 "parser.y"
    { (yyval.id) = (char *) "swig"; }
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 2108 "parser.y"
    {
                SwigType *t = (yyvsp[(2) - (4)].decl).type;
		Hash *kws = NewHash();
		String *fixname;
		fixname = feature_identifier_fix((yyvsp[(2) - (4)].decl).id);
		Setattr(kws,"name",(yyvsp[(3) - (4)].id));
		if (!Len(t)) t = 0;
		/* Special declarator check */
		if (t) {
		  if (SwigType_isfunction(t)) {
		    SwigType *decl = SwigType_pop_function(t);
		    if (SwigType_ispointer(t)) {
		      String *nname = NewStringf("*%s",fixname);
		      if ((yyvsp[(1) - (4)].intvalue)) {
			Swig_name_rename_add(Namespaceprefix, nname,decl,kws,(yyvsp[(2) - (4)].decl).parms);
		      } else {
			Swig_name_namewarn_add(Namespaceprefix,nname,decl,kws);
		      }
		      Delete(nname);
		    } else {
		      if ((yyvsp[(1) - (4)].intvalue)) {
			Swig_name_rename_add(Namespaceprefix,(fixname),decl,kws,(yyvsp[(2) - (4)].decl).parms);
		      } else {
			Swig_name_namewarn_add(Namespaceprefix,(fixname),decl,kws);
		      }
		    }
		    Delete(decl);
		  } else if (SwigType_ispointer(t)) {
		    String *nname = NewStringf("*%s",fixname);
		    if ((yyvsp[(1) - (4)].intvalue)) {
		      Swig_name_rename_add(Namespaceprefix,(nname),0,kws,(yyvsp[(2) - (4)].decl).parms);
		    } else {
		      Swig_name_namewarn_add(Namespaceprefix,(nname),0,kws);
		    }
		    Delete(nname);
		  }
		} else {
		  if ((yyvsp[(1) - (4)].intvalue)) {
		    Swig_name_rename_add(Namespaceprefix,(fixname),0,kws,(yyvsp[(2) - (4)].decl).parms);
		  } else {
		    Swig_name_namewarn_add(Namespaceprefix,(fixname),0,kws);
		  }
		}
                (yyval.node) = 0;
		scanner_clear_rename();
              }
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 2154 "parser.y"
    {
		String *fixname;
		Hash *kws = (yyvsp[(3) - (7)].node);
		SwigType *t = (yyvsp[(5) - (7)].decl).type;
		fixname = feature_identifier_fix((yyvsp[(5) - (7)].decl).id);
		if (!Len(t)) t = 0;
		/* Special declarator check */
		if (t) {
		  if ((yyvsp[(6) - (7)].dtype).qualifier) SwigType_push(t,(yyvsp[(6) - (7)].dtype).qualifier);
		  if (SwigType_isfunction(t)) {
		    SwigType *decl = SwigType_pop_function(t);
		    if (SwigType_ispointer(t)) {
		      String *nname = NewStringf("*%s",fixname);
		      if ((yyvsp[(1) - (7)].intvalue)) {
			Swig_name_rename_add(Namespaceprefix, nname,decl,kws,(yyvsp[(5) - (7)].decl).parms);
		      } else {
			Swig_name_namewarn_add(Namespaceprefix,nname,decl,kws);
		      }
		      Delete(nname);
		    } else {
		      if ((yyvsp[(1) - (7)].intvalue)) {
			Swig_name_rename_add(Namespaceprefix,(fixname),decl,kws,(yyvsp[(5) - (7)].decl).parms);
		      } else {
			Swig_name_namewarn_add(Namespaceprefix,(fixname),decl,kws);
		      }
		    }
		    Delete(decl);
		  } else if (SwigType_ispointer(t)) {
		    String *nname = NewStringf("*%s",fixname);
		    if ((yyvsp[(1) - (7)].intvalue)) {
		      Swig_name_rename_add(Namespaceprefix,(nname),0,kws,(yyvsp[(5) - (7)].decl).parms);
		    } else {
		      Swig_name_namewarn_add(Namespaceprefix,(nname),0,kws);
		    }
		    Delete(nname);
		  }
		} else {
		  if ((yyvsp[(1) - (7)].intvalue)) {
		    Swig_name_rename_add(Namespaceprefix,(fixname),0,kws,(yyvsp[(5) - (7)].decl).parms);
		  } else {
		    Swig_name_namewarn_add(Namespaceprefix,(fixname),0,kws);
		  }
		}
                (yyval.node) = 0;
		scanner_clear_rename();
              }
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 2200 "parser.y"
    {
		if ((yyvsp[(1) - (6)].intvalue)) {
		  Swig_name_rename_add(Namespaceprefix,(yyvsp[(5) - (6)].id),0,(yyvsp[(3) - (6)].node),0);
		} else {
		  Swig_name_namewarn_add(Namespaceprefix,(yyvsp[(5) - (6)].id),0,(yyvsp[(3) - (6)].node));
		}
		(yyval.node) = 0;
		scanner_clear_rename();
              }
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 2211 "parser.y"
    {
		    (yyval.intvalue) = 1;
                }
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 2214 "parser.y"
    {
                    (yyval.intvalue) = 0;
                }
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 2241 "parser.y"
    {
                    String *val = (yyvsp[(7) - (7)].str) ? NewString((yyvsp[(7) - (7)].str)) : NewString("1");
                    new_feature((yyvsp[(3) - (7)].id), val, 0, (yyvsp[(5) - (7)].decl).id, (yyvsp[(5) - (7)].decl).type, (yyvsp[(5) - (7)].decl).parms, (yyvsp[(6) - (7)].dtype).qualifier);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 2247 "parser.y"
    {
                    String *val = Len((yyvsp[(5) - (9)].id)) ? NewString((yyvsp[(5) - (9)].id)) : 0;
                    new_feature((yyvsp[(3) - (9)].id), val, 0, (yyvsp[(7) - (9)].decl).id, (yyvsp[(7) - (9)].decl).type, (yyvsp[(7) - (9)].decl).parms, (yyvsp[(8) - (9)].dtype).qualifier);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 2253 "parser.y"
    {
                    String *val = (yyvsp[(8) - (8)].str) ? NewString((yyvsp[(8) - (8)].str)) : NewString("1");
                    new_feature((yyvsp[(3) - (8)].id), val, (yyvsp[(4) - (8)].node), (yyvsp[(6) - (8)].decl).id, (yyvsp[(6) - (8)].decl).type, (yyvsp[(6) - (8)].decl).parms, (yyvsp[(7) - (8)].dtype).qualifier);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 2259 "parser.y"
    {
                    String *val = Len((yyvsp[(5) - (10)].id)) ? NewString((yyvsp[(5) - (10)].id)) : 0;
                    new_feature((yyvsp[(3) - (10)].id), val, (yyvsp[(6) - (10)].node), (yyvsp[(8) - (10)].decl).id, (yyvsp[(8) - (10)].decl).type, (yyvsp[(8) - (10)].decl).parms, (yyvsp[(9) - (10)].dtype).qualifier);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 2267 "parser.y"
    {
                    String *val = (yyvsp[(5) - (5)].str) ? NewString((yyvsp[(5) - (5)].str)) : NewString("1");
                    new_feature((yyvsp[(3) - (5)].id), val, 0, 0, 0, 0, 0);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 2273 "parser.y"
    {
                    String *val = Len((yyvsp[(5) - (7)].id)) ? NewString((yyvsp[(5) - (7)].id)) : 0;
                    new_feature((yyvsp[(3) - (7)].id), val, 0, 0, 0, 0, 0);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 2279 "parser.y"
    {
                    String *val = (yyvsp[(6) - (6)].str) ? NewString((yyvsp[(6) - (6)].str)) : NewString("1");
                    new_feature((yyvsp[(3) - (6)].id), val, (yyvsp[(4) - (6)].node), 0, 0, 0, 0);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 2285 "parser.y"
    {
                    String *val = Len((yyvsp[(5) - (8)].id)) ? NewString((yyvsp[(5) - (8)].id)) : 0;
                    new_feature((yyvsp[(3) - (8)].id), val, (yyvsp[(6) - (8)].node), 0, 0, 0, 0);
                    (yyval.node) = 0;
                    scanner_clear_rename();
                  }
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 2293 "parser.y"
    { (yyval.str) = (yyvsp[(1) - (1)].str); }
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 2294 "parser.y"
    { (yyval.str) = 0; }
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 2295 "parser.y"
    { (yyval.str) = (yyvsp[(3) - (5)].pl); }
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 2298 "parser.y"
    {
		  (yyval.node) = NewHash();
		  Setattr((yyval.node),"name",(yyvsp[(2) - (4)].id));
		  Setattr((yyval.node),"value",(yyvsp[(4) - (4)].id));
                }
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 2303 "parser.y"
    {
		  (yyval.node) = NewHash();
		  Setattr((yyval.node),"name",(yyvsp[(2) - (5)].id));
		  Setattr((yyval.node),"value",(yyvsp[(4) - (5)].id));
                  set_nextSibling((yyval.node),(yyvsp[(5) - (5)].node));
                }
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 2313 "parser.y"
    {
                 Parm *val;
		 String *name;
		 SwigType *t;
		 if (Namespaceprefix) name = NewStringf("%s::%s", Namespaceprefix, (yyvsp[(5) - (7)].decl).id);
		 else name = NewString((yyvsp[(5) - (7)].decl).id);
		 val = (yyvsp[(3) - (7)].pl);
		 if ((yyvsp[(5) - (7)].decl).parms) {
		   Setmeta(val,"parms",(yyvsp[(5) - (7)].decl).parms);
		 }
		 t = (yyvsp[(5) - (7)].decl).type;
		 if (!Len(t)) t = 0;
		 if (t) {
		   if ((yyvsp[(6) - (7)].dtype).qualifier) SwigType_push(t,(yyvsp[(6) - (7)].dtype).qualifier);
		   if (SwigType_isfunction(t)) {
		     SwigType *decl = SwigType_pop_function(t);
		     if (SwigType_ispointer(t)) {
		       String *nname = NewStringf("*%s",name);
		       Swig_feature_set(Swig_cparse_features(), nname, decl, "feature:varargs", val, 0);
		       Delete(nname);
		     } else {
		       Swig_feature_set(Swig_cparse_features(), name, decl, "feature:varargs", val, 0);
		     }
		     Delete(decl);
		   } else if (SwigType_ispointer(t)) {
		     String *nname = NewStringf("*%s",name);
		     Swig_feature_set(Swig_cparse_features(),nname,0,"feature:varargs",val, 0);
		     Delete(nname);
		   }
		 } else {
		   Swig_feature_set(Swig_cparse_features(),name,0,"feature:varargs",val, 0);
		 }
		 Delete(name);
		 (yyval.node) = 0;
              }
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 2349 "parser.y"
    { (yyval.pl) = (yyvsp[(1) - (1)].pl); }
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 2350 "parser.y"
    { 
		  int i;
		  int n;
		  Parm *p;
		  n = atoi(Char((yyvsp[(1) - (3)].dtype).val));
		  if (n <= 0) {
		    Swig_error(cparse_file, cparse_line,"Argument count in %%varargs must be positive.\n");
		    (yyval.pl) = 0;
		  } else {
		    String *name = Getattr((yyvsp[(3) - (3)].p), "name");
		    (yyval.pl) = Copy((yyvsp[(3) - (3)].p));
		    if (name)
		      Setattr((yyval.pl), "name", NewStringf("%s%d", name, n));
		    for (i = 1; i < n; i++) {
		      p = Copy((yyvsp[(3) - (3)].p));
		      name = Getattr(p, "name");
		      if (name)
		        Setattr(p, "name", NewStringf("%s%d", name, n-i));
		      set_nextSibling(p,(yyval.pl));
		      Delete((yyval.pl));
		      (yyval.pl) = p;
		    }
		  }
                }
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 2385 "parser.y"
    {
		   (yyval.node) = 0;
		   if ((yyvsp[(3) - (6)].tmap).method) {
		     String *code = 0;
		     (yyval.node) = new_node("typemap");
		     Setattr((yyval.node),"method",(yyvsp[(3) - (6)].tmap).method);
		     if ((yyvsp[(3) - (6)].tmap).kwargs) {
		       ParmList *kw = (yyvsp[(3) - (6)].tmap).kwargs;
                       code = remove_block(kw, (yyvsp[(6) - (6)].str));
		       Setattr((yyval.node),"kwargs", (yyvsp[(3) - (6)].tmap).kwargs);
		     }
		     code = code ? code : NewString((yyvsp[(6) - (6)].str));
		     Setattr((yyval.node),"code", code);
		     Delete(code);
		     appendChild((yyval.node),(yyvsp[(5) - (6)].p));
		   }
	       }
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 2402 "parser.y"
    {
		 (yyval.node) = 0;
		 if ((yyvsp[(3) - (6)].tmap).method) {
		   (yyval.node) = new_node("typemap");
		   Setattr((yyval.node),"method",(yyvsp[(3) - (6)].tmap).method);
		   appendChild((yyval.node),(yyvsp[(5) - (6)].p));
		 }
	       }
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 2410 "parser.y"
    {
		   (yyval.node) = 0;
		   if ((yyvsp[(3) - (8)].tmap).method) {
		     (yyval.node) = new_node("typemapcopy");
		     Setattr((yyval.node),"method",(yyvsp[(3) - (8)].tmap).method);
		     Setattr((yyval.node),"pattern", Getattr((yyvsp[(7) - (8)].p),"pattern"));
		     appendChild((yyval.node),(yyvsp[(5) - (8)].p));
		   }
	       }
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 2423 "parser.y"
    {
		 Hash *p;
		 String *name;
		 p = nextSibling((yyvsp[(1) - (1)].node));
		 if (p && (!Getattr(p,"value"))) {
 		   /* this is the deprecated two argument typemap form */
 		   Swig_warning(WARN_DEPRECATED_TYPEMAP_LANG,cparse_file, cparse_line,
				"Specifying the language name in %%typemap is deprecated - use #ifdef SWIG<LANG> instead.\n");
		   /* two argument typemap form */
		   name = Getattr((yyvsp[(1) - (1)].node),"name");
		   if (!name || (Strcmp(name,typemap_lang))) {
		     (yyval.tmap).method = 0;
		     (yyval.tmap).kwargs = 0;
		   } else {
		     (yyval.tmap).method = Getattr(p,"name");
		     (yyval.tmap).kwargs = nextSibling(p);
		   }
		 } else {
		   /* one-argument typemap-form */
		   (yyval.tmap).method = Getattr((yyvsp[(1) - (1)].node),"name");
		   (yyval.tmap).kwargs = p;
		 }
                }
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 2448 "parser.y"
    {
                 (yyval.p) = (yyvsp[(1) - (2)].p);
		 set_nextSibling((yyval.p),(yyvsp[(2) - (2)].p));
		}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 2454 "parser.y"
    {
                 (yyval.p) = (yyvsp[(2) - (3)].p);
		 set_nextSibling((yyval.p),(yyvsp[(3) - (3)].p));
                }
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 2458 "parser.y"
    { (yyval.p) = 0;}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 2461 "parser.y"
    {
                  Parm *parm;
		  SwigType_push((yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].decl).type);
		  (yyval.p) = new_node("typemapitem");
		  parm = NewParmWithoutFileLineInfo((yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].decl).id);
		  Setattr((yyval.p),"pattern",parm);
		  Setattr((yyval.p),"parms", (yyvsp[(2) - (2)].decl).parms);
		  Delete(parm);
		  /*		  $$ = NewParmWithoutFileLineInfo($1,$2.id);
				  Setattr($$,"parms",$2.parms); */
                }
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 2472 "parser.y"
    {
                  (yyval.p) = new_node("typemapitem");
		  Setattr((yyval.p),"pattern",(yyvsp[(2) - (3)].pl));
		  /*		  Setattr($$,"multitype",$2); */
               }
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 2477 "parser.y"
    {
		 (yyval.p) = new_node("typemapitem");
		 Setattr((yyval.p),"pattern", (yyvsp[(2) - (6)].pl));
		 /*                 Setattr($$,"multitype",$2); */
		 Setattr((yyval.p),"parms",(yyvsp[(5) - (6)].pl));
               }
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 2490 "parser.y"
    {
                   (yyval.node) = new_node("types");
		   Setattr((yyval.node),"parms",(yyvsp[(3) - (5)].pl));
                   if ((yyvsp[(5) - (5)].str))
		     Setattr((yyval.node),"convcode",NewString((yyvsp[(5) - (5)].str)));
               }
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 2502 "parser.y"
    {
                  Parm *p, *tp;
		  Node *n;
		  Symtab *tscope = 0;
		  int     specialized = 0;
		  int     variadic = 0;

		  (yyval.node) = 0;

		  tscope = Swig_symbol_current();          /* Get the current scope */

		  /* If the class name is qualified, we need to create or lookup namespace entries */
		  if (!inclass) {
		    (yyvsp[(5) - (9)].str) = resolve_create_node_scope((yyvsp[(5) - (9)].str));
		  }

		  /*
		    We use the new namespace entry 'nscope' only to
		    emit the template node. The template parameters are
		    resolved in the current 'tscope'.

		    This is closer to the C++ (typedef) behavior.
		  */
		  n = Swig_cparse_template_locate((yyvsp[(5) - (9)].str),(yyvsp[(7) - (9)].p),tscope);

		  /* Patch the argument types to respect namespaces */
		  p = (yyvsp[(7) - (9)].p);
		  while (p) {
		    SwigType *value = Getattr(p,"value");
		    if (!value) {
		      SwigType *ty = Getattr(p,"type");
		      if (ty) {
			SwigType *rty = 0;
			int reduce = template_reduce;
			if (reduce || !SwigType_ispointer(ty)) {
			  rty = Swig_symbol_typedef_reduce(ty,tscope);
			  if (!reduce) reduce = SwigType_ispointer(rty);
			}
			ty = reduce ? Swig_symbol_type_qualify(rty,tscope) : Swig_symbol_type_qualify(ty,tscope);
			Setattr(p,"type",ty);
			Delete(ty);
			Delete(rty);
		      }
		    } else {
		      value = Swig_symbol_type_qualify(value,tscope);
		      Setattr(p,"value",value);
		      Delete(value);
		    }

		    p = nextSibling(p);
		  }

		  /* Look for the template */
		  {
                    Node *nn = n;
                    Node *linklistend = 0;
                    while (nn) {
                      Node *templnode = 0;
                      if (Strcmp(nodeType(nn),"template") == 0) {
                        int nnisclass = (Strcmp(Getattr(nn,"templatetype"),"class") == 0); /* if not a templated class it is a templated function */
                        Parm *tparms = Getattr(nn,"templateparms");
                        if (!tparms) {
                          specialized = 1;
                        } else if (Getattr(tparms,"variadic") && strncmp(Char(Getattr(tparms,"variadic")), "1", 1)==0) {
                          variadic = 1;
                        }
                        if (nnisclass && !variadic && !specialized && (ParmList_len((yyvsp[(7) - (9)].p)) > ParmList_len(tparms))) {
                          Swig_error(cparse_file, cparse_line, "Too many template parameters. Maximum of %d.\n", ParmList_len(tparms));
                        } else if (nnisclass && !specialized && ((ParmList_len((yyvsp[(7) - (9)].p)) < (ParmList_numrequired(tparms) - (variadic?1:0))))) { /* Variadic parameter is optional */
                          Swig_error(cparse_file, cparse_line, "Not enough template parameters specified. %d required.\n", (ParmList_numrequired(tparms)-(variadic?1:0)) );
                        } else if (!nnisclass && ((ParmList_len((yyvsp[(7) - (9)].p)) != ParmList_len(tparms)))) {
                          /* must be an overloaded templated method - ignore it as it is overloaded with a different number of template parameters */
                          nn = Getattr(nn,"sym:nextSibling"); /* repeat for overloaded templated functions */
                          continue;
                        } else {
			  String *tname = Copy((yyvsp[(5) - (9)].str));
                          int def_supplied = 0;
                          /* Expand the template */
			  Node *templ = Swig_symbol_clookup((yyvsp[(5) - (9)].str),0);
			  Parm *targs = templ ? Getattr(templ,"templateparms") : 0;

                          ParmList *temparms;
                          if (specialized) temparms = CopyParmList((yyvsp[(7) - (9)].p));
                          else temparms = CopyParmList(tparms);

                          /* Create typedef's and arguments */
                          p = (yyvsp[(7) - (9)].p);
                          tp = temparms;
                          if (!p && ParmList_len(p) != ParmList_len(temparms)) {
                            /* we have no template parameters supplied in %template for a template that has default args*/
                            p = tp;
                            def_supplied = 1;
                          }

                          while (p) {
                            String *value = Getattr(p,"value");
                            if (def_supplied) {
                              Setattr(p,"default","1");
                            }
                            if (value) {
                              Setattr(tp,"value",value);
                            } else {
                              SwigType *ty = Getattr(p,"type");
                              if (ty) {
                                Setattr(tp,"type",ty);
                              }
                              Delattr(tp,"value");
                            }
			    /* fix default arg values */
			    if (targs) {
			      Parm *pi = temparms;
			      Parm *ti = targs;
			      String *tv = Getattr(tp,"value");
			      if (!tv) tv = Getattr(tp,"type");
			      while(pi != tp && ti && pi) {
				String *name = Getattr(ti,"name");
				String *value = Getattr(pi,"value");
				if (!value) value = Getattr(pi,"type");
				Replaceid(tv, name, value);
				pi = nextSibling(pi);
				ti = nextSibling(ti);
			      }
			    }
                            p = nextSibling(p);
                            tp = nextSibling(tp);
                            if (!p && tp) {
                              p = tp;
                              def_supplied = 1;
                            } else if (p && !tp) { /* Variadic template - tp < p */
			      SWIG_WARN_NODE_BEGIN(nn);
                              Swig_warning(WARN_CPP11_VARIADIC_TEMPLATE,cparse_file, cparse_line,"Only the first variadic template argument is currently supported.\n");
			      SWIG_WARN_NODE_END(nn);
                              break;
                            }
                          }

                          templnode = copy_node(nn);
			  update_nested_classes(templnode); /* update classes nested withing template */
                          /* We need to set the node name based on name used to instantiate */
                          Setattr(templnode,"name",tname);
			  Delete(tname);
                          if (!specialized) {
                            Delattr(templnode,"sym:typename");
                          } else {
                            Setattr(templnode,"sym:typename","1");
                          }
			  /* for now, nested %template is allowed only in the same scope as the template declaration */
                          if ((yyvsp[(3) - (9)].id) && !(nnisclass && ((currentOuterClass && (currentOuterClass != Getattr(nn, "nested:outer")))
			    ||(extendmode && current_class && (current_class != Getattr(nn, "nested:outer")))))) {
			    /*
			       Comment this out for 1.3.28. We need to
			       re-enable it later but first we need to
			       move %ignore from using %rename to use
			       %feature(ignore).

			       String *symname = Swig_name_make(templnode,0,$3,0,0);
			    */
			    String *symname = (yyvsp[(3) - (9)].id);
                            Swig_cparse_template_expand(templnode,symname,temparms,tscope);
                            Setattr(templnode,"sym:name",symname);
                          } else {
                            static int cnt = 0;
                            String *nname = NewStringf("__dummy_%d__", cnt++);
                            Swig_cparse_template_expand(templnode,nname,temparms,tscope);
                            Setattr(templnode,"sym:name",nname);
			    Delete(nname);
                            Setattr(templnode,"feature:onlychildren", "typemap,typemapitem,typemapcopy,typedef,types,fragment");
			    if ((yyvsp[(3) - (9)].id)) {
			      Swig_warning(WARN_PARSE_NESTED_TEMPLATE, cparse_file, cparse_line, "Named nested template instantiations not supported. Processing as if no name was given to %%template().\n");
			    }
                          }
                          Delattr(templnode,"templatetype");
                          Setattr(templnode,"template",nn);
                          Setfile(templnode,cparse_file);
                          Setline(templnode,cparse_line);
                          Delete(temparms);
			  if (currentOuterClass) {
			    SetFlag(templnode, "nested");
			    Setattr(templnode, "nested:outer", currentOuterClass);
			  }
                          add_symbols_copy(templnode);

                          if (Strcmp(nodeType(templnode),"class") == 0) {

                            /* Identify pure abstract methods */
                            Setattr(templnode,"abstracts", pure_abstracts(firstChild(templnode)));

                            /* Set up inheritance in symbol table */
                            {
                              Symtab  *csyms;
                              List *baselist = Getattr(templnode,"baselist");
                              csyms = Swig_symbol_current();
                              Swig_symbol_setscope(Getattr(templnode,"symtab"));
                              if (baselist) {
                                List *bases = Swig_make_inherit_list(Getattr(templnode,"name"),baselist, Namespaceprefix);
                                if (bases) {
                                  Iterator s;
                                  for (s = First(bases); s.item; s = Next(s)) {
                                    Symtab *st = Getattr(s.item,"symtab");
                                    if (st) {
				      Setfile(st,Getfile(s.item));
				      Setline(st,Getline(s.item));
                                      Swig_symbol_inherit(st);
                                    }
                                  }
				  Delete(bases);
                                }
                              }
                              Swig_symbol_setscope(csyms);
                            }

                            /* Merge in %extend methods for this class */

			    /* !!! This may be broken.  We may have to add the
			       %extend methods at the beginning of the class */
                            {
                              String *stmp = 0;
                              String *clsname;
                              Node *am;
                              if (Namespaceprefix) {
                                clsname = stmp = NewStringf("%s::%s", Namespaceprefix, Getattr(templnode,"name"));
                              } else {
                                clsname = Getattr(templnode,"name");
                              }
                              am = Getattr(Swig_extend_hash(),clsname);
                              if (am) {
                                Symtab *st = Swig_symbol_current();
                                Swig_symbol_setscope(Getattr(templnode,"symtab"));
                                /*			    Printf(stdout,"%s: %s %p %p\n", Getattr(templnode,"name"), clsname, Swig_symbol_current(), Getattr(templnode,"symtab")); */
                                Swig_extend_merge(templnode,am);
                                Swig_symbol_setscope(st);
				Swig_extend_append_previous(templnode,am);
                                Delattr(Swig_extend_hash(),clsname);
                              }
			      if (stmp) Delete(stmp);
                            }

                            /* Add to classes hash */
			    if (!classes)
			      classes = NewHash();

			    if (Namespaceprefix) {
			      String *temp = NewStringf("%s::%s", Namespaceprefix, Getattr(templnode,"name"));
			      Setattr(classes,temp,templnode);
			      Delete(temp);
			    } else {
			      String *qs = Swig_symbol_qualifiedscopename(templnode);
			      Setattr(classes, qs,templnode);
			      Delete(qs);
			    }
                          }
                        }

                        /* all the overloaded templated functions are added into a linked list */
                        if (nscope_inner) {
                          /* non-global namespace */
                          if (templnode) {
                            appendChild(nscope_inner,templnode);
			    Delete(templnode);
                            if (nscope) (yyval.node) = nscope;
                          }
                        } else {
                          /* global namespace */
                          if (!linklistend) {
                            (yyval.node) = templnode;
                          } else {
                            set_nextSibling(linklistend,templnode);
			    Delete(templnode);
                          }
                          linklistend = templnode;
                        }
                      }
                      nn = Getattr(nn,"sym:nextSibling"); /* repeat for overloaded templated functions. If a templated class there will never be a sibling. */
                    }
		  }
	          Swig_symbol_setscope(tscope);
		  Delete(Namespaceprefix);
		  Namespaceprefix = Swig_symbol_qualifiedscopename(0);
                }
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 2788 "parser.y"
    {
		  Swig_warning(0,cparse_file, cparse_line,"%s\n", (yyvsp[(2) - (2)].id));
		  (yyval.node) = 0;
               }
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 2798 "parser.y"
    {
                    (yyval.node) = (yyvsp[(1) - (1)].node); 
                    if ((yyval.node)) {
   		      add_symbols((yyval.node));
                      default_arguments((yyval.node));
   	            }
                }
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 2805 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 2806 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 2810 "parser.y"
    {
		  if (Strcmp((yyvsp[(2) - (3)].id),"C") == 0) {
		    cparse_externc = 1;
		  }
		}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 2814 "parser.y"
    {
		  cparse_externc = 0;
		  if (Strcmp((yyvsp[(2) - (6)].id),"C") == 0) {
		    Node *n = firstChild((yyvsp[(5) - (6)].node));
		    (yyval.node) = new_node("extern");
		    Setattr((yyval.node),"name",(yyvsp[(2) - (6)].id));
		    appendChild((yyval.node),n);
		    while (n) {
		      SwigType *decl = Getattr(n,"decl");
		      if (SwigType_isfunction(decl) && !Equal(Getattr(n, "storage"), "typedef")) {
			Setattr(n,"storage","externc");
		      }
		      n = nextSibling(n);
		    }
		  } else {
		     Swig_warning(WARN_PARSE_UNDEFINED_EXTERN,cparse_file, cparse_line,"Unrecognized extern type \"%s\".\n", (yyvsp[(2) - (6)].id));
		    (yyval.node) = new_node("extern");
		    Setattr((yyval.node),"name",(yyvsp[(2) - (6)].id));
		    appendChild((yyval.node),firstChild((yyvsp[(5) - (6)].node)));
		  }
                }
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 2835 "parser.y"
    {
		  (yyval.node) = (yyvsp[(1) - (1)].node);
		  SWIG_WARN_NODE_BEGIN((yyval.node));
		  Swig_warning(WARN_CPP11_LAMBDA, cparse_file, cparse_line, "Lambda expressions and closures are not fully supported yet.\n");
		  SWIG_WARN_NODE_END((yyval.node));
		}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 2841 "parser.y"
    {
		  skip_decl();
		  (yyval.node) = new_node("using");
		  Setattr((yyval.node),"name",(yyvsp[(2) - (3)].str));
		  add_symbols((yyval.node));
		  SWIG_WARN_NODE_BEGIN((yyval.node));
		  Swig_warning(WARN_CPP11_ALIAS_DECLARATION, cparse_file, cparse_line, "The 'using' keyword in type aliasing is not fully supported yet.\n");
		  SWIG_WARN_NODE_END((yyval.node));

		  (yyval.node) = 0; /* TODO - ignored for now */
		}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 2852 "parser.y"
    {
		  skip_decl();
		  (yyval.node) = new_node("using");
		  Setattr((yyval.node),"uname",(yyvsp[(8) - (8)].id));
		  Setattr((yyval.node),"name",(yyvsp[(6) - (8)].str));
		  add_symbols((yyval.node));
		  SWIG_WARN_NODE_BEGIN((yyval.node));
		  Swig_warning(WARN_CPP11_ALIAS_TEMPLATE, cparse_file, cparse_line, "The 'using' keyword in template aliasing is not fully supported yet.\n");
		  SWIG_WARN_NODE_END((yyval.node));
		}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 2868 "parser.y"
    {
              (yyval.node) = new_node("cdecl");
	      if ((yyvsp[(4) - (5)].dtype).qualifier) SwigType_push((yyvsp[(3) - (5)].decl).type,(yyvsp[(4) - (5)].dtype).qualifier);
	      Setattr((yyval.node),"type",(yyvsp[(2) - (5)].type));
	      Setattr((yyval.node),"storage",(yyvsp[(1) - (5)].id));
	      Setattr((yyval.node),"name",(yyvsp[(3) - (5)].decl).id);
	      Setattr((yyval.node),"decl",(yyvsp[(3) - (5)].decl).type);
	      Setattr((yyval.node),"parms",(yyvsp[(3) - (5)].decl).parms);
	      Setattr((yyval.node),"value",(yyvsp[(4) - (5)].dtype).val);
	      Setattr((yyval.node),"throws",(yyvsp[(4) - (5)].dtype).throws);
	      Setattr((yyval.node),"throw",(yyvsp[(4) - (5)].dtype).throwf);
	      Setattr((yyval.node),"noexcept",(yyvsp[(4) - (5)].dtype).nexcept);
	      if (!(yyvsp[(5) - (5)].node)) {
		if (Len(scanner_ccode)) {
		  String *code = Copy(scanner_ccode);
		  Setattr((yyval.node),"code",code);
		  Delete(code);
		}
	      } else {
		Node *n = (yyvsp[(5) - (5)].node);
		/* Inherit attributes */
		while (n) {
		  String *type = Copy((yyvsp[(2) - (5)].type));
		  Setattr(n,"type",type);
		  Setattr(n,"storage",(yyvsp[(1) - (5)].id));
		  n = nextSibling(n);
		  Delete(type);
		}
	      }
	      if ((yyvsp[(4) - (5)].dtype).bitfield) {
		Setattr((yyval.node),"bitfield", (yyvsp[(4) - (5)].dtype).bitfield);
	      }

	      /* Look for "::" declarations (ignored) */
	      if (Strstr((yyvsp[(3) - (5)].decl).id,"::")) {
                /* This is a special case. If the scope name of the declaration exactly
                   matches that of the declaration, then we will allow it. Otherwise, delete. */
                String *p = Swig_scopename_prefix((yyvsp[(3) - (5)].decl).id);
		if (p) {
		  if ((Namespaceprefix && Strcmp(p,Namespaceprefix) == 0) ||
		      (inclass && Strcmp(p,Classprefix) == 0)) {
		    String *lstr = Swig_scopename_last((yyvsp[(3) - (5)].decl).id);
		    Setattr((yyval.node),"name",lstr);
		    Delete(lstr);
		    set_nextSibling((yyval.node),(yyvsp[(5) - (5)].node));
		  } else {
		    Delete((yyval.node));
		    (yyval.node) = (yyvsp[(5) - (5)].node);
		  }
		  Delete(p);
		} else {
		  Delete((yyval.node));
		  (yyval.node) = (yyvsp[(5) - (5)].node);
		}
	      } else {
		set_nextSibling((yyval.node),(yyvsp[(5) - (5)].node));
	      }
           }
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 2928 "parser.y"
    {
              (yyval.node) = new_node("cdecl");
	      if ((yyvsp[(6) - (7)].dtype).qualifier) SwigType_push((yyvsp[(3) - (7)].decl).type,(yyvsp[(6) - (7)].dtype).qualifier);
	      Setattr((yyval.node),"type",(yyvsp[(5) - (7)].node));
	      Setattr((yyval.node),"storage",(yyvsp[(1) - (7)].id));
	      Setattr((yyval.node),"name",(yyvsp[(3) - (7)].decl).id);
	      Setattr((yyval.node),"decl",(yyvsp[(3) - (7)].decl).type);
	      Setattr((yyval.node),"parms",(yyvsp[(3) - (7)].decl).parms);
	      Setattr((yyval.node),"value",(yyvsp[(6) - (7)].dtype).val);
	      Setattr((yyval.node),"throws",(yyvsp[(6) - (7)].dtype).throws);
	      Setattr((yyval.node),"throw",(yyvsp[(6) - (7)].dtype).throwf);
	      Setattr((yyval.node),"noexcept",(yyvsp[(6) - (7)].dtype).nexcept);
	      if (!(yyvsp[(7) - (7)].node)) {
		if (Len(scanner_ccode)) {
		  String *code = Copy(scanner_ccode);
		  Setattr((yyval.node),"code",code);
		  Delete(code);
		}
	      } else {
		Node *n = (yyvsp[(7) - (7)].node);
		while (n) {
		  String *type = Copy((yyvsp[(5) - (7)].node));
		  Setattr(n,"type",type);
		  Setattr(n,"storage",(yyvsp[(1) - (7)].id));
		  n = nextSibling(n);
		  Delete(type);
		}
	      }
	      if ((yyvsp[(6) - (7)].dtype).bitfield) {
		Setattr((yyval.node),"bitfield", (yyvsp[(6) - (7)].dtype).bitfield);
	      }

	      if (Strstr((yyvsp[(3) - (7)].decl).id,"::")) {
                String *p = Swig_scopename_prefix((yyvsp[(3) - (7)].decl).id);
		if (p) {
		  if ((Namespaceprefix && Strcmp(p,Namespaceprefix) == 0) ||
		      (inclass && Strcmp(p,Classprefix) == 0)) {
		    String *lstr = Swig_scopename_last((yyvsp[(3) - (7)].decl).id);
		    Setattr((yyval.node),"name",lstr);
		    Delete(lstr);
		    set_nextSibling((yyval.node),(yyvsp[(7) - (7)].node));
		  } else {
		    Delete((yyval.node));
		    (yyval.node) = (yyvsp[(7) - (7)].node);
		  }
		  Delete(p);
		} else {
		  Delete((yyval.node));
		  (yyval.node) = (yyvsp[(7) - (7)].node);
		}
	      } else {
		set_nextSibling((yyval.node),(yyvsp[(7) - (7)].node));
	      }
           }
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 2986 "parser.y"
    { 
                   (yyval.node) = 0;
                   Clear(scanner_ccode); 
               }
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 2990 "parser.y"
    {
		 (yyval.node) = new_node("cdecl");
		 if ((yyvsp[(3) - (4)].dtype).qualifier) SwigType_push((yyvsp[(2) - (4)].decl).type,(yyvsp[(3) - (4)].dtype).qualifier);
		 Setattr((yyval.node),"name",(yyvsp[(2) - (4)].decl).id);
		 Setattr((yyval.node),"decl",(yyvsp[(2) - (4)].decl).type);
		 Setattr((yyval.node),"parms",(yyvsp[(2) - (4)].decl).parms);
		 Setattr((yyval.node),"value",(yyvsp[(3) - (4)].dtype).val);
		 Setattr((yyval.node),"throws",(yyvsp[(3) - (4)].dtype).throws);
		 Setattr((yyval.node),"throw",(yyvsp[(3) - (4)].dtype).throwf);
		 Setattr((yyval.node),"noexcept",(yyvsp[(3) - (4)].dtype).nexcept);
		 if ((yyvsp[(3) - (4)].dtype).bitfield) {
		   Setattr((yyval.node),"bitfield", (yyvsp[(3) - (4)].dtype).bitfield);
		 }
		 if (!(yyvsp[(4) - (4)].node)) {
		   if (Len(scanner_ccode)) {
		     String *code = Copy(scanner_ccode);
		     Setattr((yyval.node),"code",code);
		     Delete(code);
		   }
		 } else {
		   set_nextSibling((yyval.node),(yyvsp[(4) - (4)].node));
		 }
	       }
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 3013 "parser.y"
    { 
                   skip_balanced('{','}');
                   (yyval.node) = 0;
               }
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 3019 "parser.y"
    { 
                   (yyval.dtype) = (yyvsp[(1) - (1)].dtype); 
                   (yyval.dtype).qualifier = 0;
		   (yyval.dtype).throws = 0;
		   (yyval.dtype).throwf = 0;
		   (yyval.dtype).nexcept = 0;
              }
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 3026 "parser.y"
    { 
                   (yyval.dtype) = (yyvsp[(2) - (2)].dtype); 
		   (yyval.dtype).qualifier = (yyvsp[(1) - (2)].str);
		   (yyval.dtype).throws = 0;
		   (yyval.dtype).throwf = 0;
		   (yyval.dtype).nexcept = 0;
	      }
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 3033 "parser.y"
    { 
		   (yyval.dtype) = (yyvsp[(2) - (2)].dtype); 
                   (yyval.dtype).qualifier = 0;
		   (yyval.dtype).throws = (yyvsp[(1) - (2)].dtype).throws;
		   (yyval.dtype).throwf = (yyvsp[(1) - (2)].dtype).throwf;
		   (yyval.dtype).nexcept = (yyvsp[(1) - (2)].dtype).nexcept;
              }
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 3040 "parser.y"
    { 
                   (yyval.dtype) = (yyvsp[(3) - (3)].dtype); 
                   (yyval.dtype).qualifier = (yyvsp[(1) - (3)].str);
		   (yyval.dtype).throws = (yyvsp[(2) - (3)].dtype).throws;
		   (yyval.dtype).throwf = (yyvsp[(2) - (3)].dtype).throwf;
		   (yyval.dtype).nexcept = (yyvsp[(2) - (3)].dtype).nexcept;
              }
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 3049 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].type); }
    break;

  case 131:

/* Line 1806 of yacc.c  */
#line 3050 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].type); }
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 3051 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].type); }
    break;

  case 133:

/* Line 1806 of yacc.c  */
#line 3052 "parser.y"
    { (yyval.node) = NewStringf("%s%s",(yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].id)); }
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 3053 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].type); }
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 3054 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].str); }
    break;

  case 136:

/* Line 1806 of yacc.c  */
#line 3055 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].type); }
    break;

  case 137:

/* Line 1806 of yacc.c  */
#line 3066 "parser.y"
    {
		  (yyval.node) = new_node("lambda");
		  Setattr((yyval.node),"name",(yyvsp[(3) - (11)].str));
		  add_symbols((yyval.node));
	        }
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 3071 "parser.y"
    {
		  (yyval.node) = new_node("lambda");
		  Setattr((yyval.node),"name",(yyvsp[(3) - (13)].str));
		  add_symbols((yyval.node));
		}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 3076 "parser.y"
    {
		  (yyval.node) = new_node("lambda");
		  Setattr((yyval.node),"name",(yyvsp[(3) - (7)].str));
		  add_symbols((yyval.node));
		}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 3083 "parser.y"
    {
		  skip_balanced('[',']');
		  (yyval.node) = 0;
	        }
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 3089 "parser.y"
    {
		  skip_balanced('{','}');
		  (yyval.node) = 0;
		}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 3094 "parser.y"
    {
		  (yyval.pl) = 0;
		}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 3097 "parser.y"
    {
		  skip_balanced('(',')');
		}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 3099 "parser.y"
    {
		  (yyval.pl) = 0;
		}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 3110 "parser.y"
    {
		   (yyval.node) = (char *)"enum";
	      }
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 3113 "parser.y"
    {
		   (yyval.node) = (char *)"enum class";
	      }
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 3116 "parser.y"
    {
		   (yyval.node) = (char *)"enum struct";
	      }
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 3125 "parser.y"
    {
                   (yyval.node) = (yyvsp[(2) - (2)].type);
              }
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 3128 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 3135 "parser.y"
    {
		   SwigType *ty = 0;
		   int scopedenum = (yyvsp[(3) - (5)].id) && !Equal((yyvsp[(2) - (5)].node), "enum");
		   (yyval.node) = new_node("enumforward");
		   ty = NewStringf("enum %s", (yyvsp[(3) - (5)].id));
		   Setattr((yyval.node),"enumkey",(yyvsp[(2) - (5)].node));
		   if (scopedenum)
		     SetFlag((yyval.node), "scopedenum");
		   Setattr((yyval.node),"name",(yyvsp[(3) - (5)].id));
		   Setattr((yyval.node),"inherit",(yyvsp[(4) - (5)].node));
		   Setattr((yyval.node),"type",ty);
		   Setattr((yyval.node),"sym:weak", "1");
		   add_symbols((yyval.node));
	      }
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 3157 "parser.y"
    {
		  SwigType *ty = 0;
		  int scopedenum = (yyvsp[(3) - (8)].id) && !Equal((yyvsp[(2) - (8)].node), "enum");
                  (yyval.node) = new_node("enum");
		  ty = NewStringf("enum %s", (yyvsp[(3) - (8)].id));
		  Setattr((yyval.node),"enumkey",(yyvsp[(2) - (8)].node));
		  if (scopedenum)
		    SetFlag((yyval.node), "scopedenum");
		  Setattr((yyval.node),"name",(yyvsp[(3) - (8)].id));
		  Setattr((yyval.node),"inherit",(yyvsp[(4) - (8)].node));
		  Setattr((yyval.node),"type",ty);
		  appendChild((yyval.node),(yyvsp[(6) - (8)].node));
		  add_symbols((yyval.node));      /* Add to tag space */

		  if (scopedenum) {
		    Swig_symbol_newscope();
		    Swig_symbol_setscopename((yyvsp[(3) - (8)].id));
		    Delete(Namespaceprefix);
		    Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		  }

		  add_symbols((yyvsp[(6) - (8)].node));      /* Add enum values to appropriate enum or enum class scope */

		  if (scopedenum) {
		    Setattr((yyval.node),"symtab", Swig_symbol_popscope());
		    Delete(Namespaceprefix);
		    Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		  }
               }
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 3186 "parser.y"
    {
		 Node *n;
		 SwigType *ty = 0;
		 String   *unnamed = 0;
		 int       unnamedinstance = 0;
		 int scopedenum = (yyvsp[(3) - (10)].id) && !Equal((yyvsp[(2) - (10)].node), "enum");

		 (yyval.node) = new_node("enum");
		 Setattr((yyval.node),"enumkey",(yyvsp[(2) - (10)].node));
		 if (scopedenum)
		   SetFlag((yyval.node), "scopedenum");
		 Setattr((yyval.node),"inherit",(yyvsp[(4) - (10)].node));
		 if ((yyvsp[(3) - (10)].id)) {
		   Setattr((yyval.node),"name",(yyvsp[(3) - (10)].id));
		   ty = NewStringf("enum %s", (yyvsp[(3) - (10)].id));
		 } else if ((yyvsp[(8) - (10)].decl).id) {
		   unnamed = make_unnamed();
		   ty = NewStringf("enum %s", unnamed);
		   Setattr((yyval.node),"unnamed",unnamed);
                   /* name is not set for unnamed enum instances, e.g. enum { foo } Instance; */
		   if ((yyvsp[(1) - (10)].id) && Cmp((yyvsp[(1) - (10)].id),"typedef") == 0) {
		     Setattr((yyval.node),"name",(yyvsp[(8) - (10)].decl).id);
                   } else {
                     unnamedinstance = 1;
                   }
		   Setattr((yyval.node),"storage",(yyvsp[(1) - (10)].id));
		 }
		 if ((yyvsp[(8) - (10)].decl).id && Cmp((yyvsp[(1) - (10)].id),"typedef") == 0) {
		   Setattr((yyval.node),"tdname",(yyvsp[(8) - (10)].decl).id);
                   Setattr((yyval.node),"allows_typedef","1");
                 }
		 appendChild((yyval.node),(yyvsp[(6) - (10)].node));
		 n = new_node("cdecl");
		 Setattr(n,"type",ty);
		 Setattr(n,"name",(yyvsp[(8) - (10)].decl).id);
		 Setattr(n,"storage",(yyvsp[(1) - (10)].id));
		 Setattr(n,"decl",(yyvsp[(8) - (10)].decl).type);
		 Setattr(n,"parms",(yyvsp[(8) - (10)].decl).parms);
		 Setattr(n,"unnamed",unnamed);

                 if (unnamedinstance) {
		   SwigType *cty = NewString("enum ");
		   Setattr((yyval.node),"type",cty);
		   SetFlag((yyval.node),"unnamedinstance");
		   SetFlag(n,"unnamedinstance");
		   Delete(cty);
                 }
		 if ((yyvsp[(10) - (10)].node)) {
		   Node *p = (yyvsp[(10) - (10)].node);
		   set_nextSibling(n,p);
		   while (p) {
		     SwigType *cty = Copy(ty);
		     Setattr(p,"type",cty);
		     Setattr(p,"unnamed",unnamed);
		     Setattr(p,"storage",(yyvsp[(1) - (10)].id));
		     Delete(cty);
		     p = nextSibling(p);
		   }
		 } else {
		   if (Len(scanner_ccode)) {
		     String *code = Copy(scanner_ccode);
		     Setattr(n,"code",code);
		     Delete(code);
		   }
		 }

                 /* Ensure that typedef enum ABC {foo} XYZ; uses XYZ for sym:name, like structs.
                  * Note that class_rename/yyrename are bit of a mess so used this simple approach to change the name. */
                 if ((yyvsp[(8) - (10)].decl).id && (yyvsp[(3) - (10)].id) && Cmp((yyvsp[(1) - (10)].id),"typedef") == 0) {
		   String *name = NewString((yyvsp[(8) - (10)].decl).id);
                   Setattr((yyval.node), "parser:makename", name);
		   Delete(name);
                 }

		 add_symbols((yyval.node));       /* Add enum to tag space */
		 set_nextSibling((yyval.node),n);
		 Delete(n);

		 if (scopedenum) {
		   Swig_symbol_newscope();
		   Swig_symbol_setscopename((yyvsp[(3) - (10)].id));
		   Delete(Namespaceprefix);
		   Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		 }

		 add_symbols((yyvsp[(6) - (10)].node));      /* Add enum values to appropriate enum or enum class scope */

		 if (scopedenum) {
		   Setattr((yyval.node),"symtab", Swig_symbol_popscope());
		   Delete(Namespaceprefix);
		   Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		 }

	         add_symbols(n);
		 Delete(unnamed);
	       }
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 3284 "parser.y"
    {
                   /* This is a sick hack.  If the ctor_end has parameters,
                      and the parms parameter only has 1 parameter, this
                      could be a declaration of the form:

                         type (id)(parms)

			 Otherwise it's an error. */
                    int err = 0;
                    (yyval.node) = 0;

		    if ((ParmList_len((yyvsp[(4) - (6)].pl)) == 1) && (!Swig_scopename_check((yyvsp[(2) - (6)].type)))) {
		      SwigType *ty = Getattr((yyvsp[(4) - (6)].pl),"type");
		      String *name = Getattr((yyvsp[(4) - (6)].pl),"name");
		      err = 1;
		      if (!name) {
			(yyval.node) = new_node("cdecl");
			Setattr((yyval.node),"type",(yyvsp[(2) - (6)].type));
			Setattr((yyval.node),"storage",(yyvsp[(1) - (6)].id));
			Setattr((yyval.node),"name",ty);

			if ((yyvsp[(6) - (6)].decl).have_parms) {
			  SwigType *decl = NewStringEmpty();
			  SwigType_add_function(decl,(yyvsp[(6) - (6)].decl).parms);
			  Setattr((yyval.node),"decl",decl);
			  Setattr((yyval.node),"parms",(yyvsp[(6) - (6)].decl).parms);
			  if (Len(scanner_ccode)) {
			    String *code = Copy(scanner_ccode);
			    Setattr((yyval.node),"code",code);
			    Delete(code);
			  }
			}
			if ((yyvsp[(6) - (6)].decl).defarg) {
			  Setattr((yyval.node),"value",(yyvsp[(6) - (6)].decl).defarg);
			}
			Setattr((yyval.node),"throws",(yyvsp[(6) - (6)].decl).throws);
			Setattr((yyval.node),"throw",(yyvsp[(6) - (6)].decl).throwf);
			Setattr((yyval.node),"noexcept",(yyvsp[(6) - (6)].decl).nexcept);
			err = 0;
		      }
		    }
		    if (err) {
		      Swig_error(cparse_file,cparse_line,"Syntax error in input(2).\n");
		      exit(1);
		    }
                }
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 3336 "parser.y"
    {  (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 3337 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 3338 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 3339 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 3340 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 3341 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 3346 "parser.y"
    {
                   String *prefix;
                   List *bases = 0;
		   Node *scope = 0;
		   String *code;
		   (yyval.node) = new_node("class");
		   Setline((yyval.node),cparse_start_line);
		   Setattr((yyval.node),"kind",(yyvsp[(2) - (5)].id));
		   if ((yyvsp[(4) - (5)].bases)) {
		     Setattr((yyval.node),"baselist", Getattr((yyvsp[(4) - (5)].bases),"public"));
		     Setattr((yyval.node),"protectedbaselist", Getattr((yyvsp[(4) - (5)].bases),"protected"));
		     Setattr((yyval.node),"privatebaselist", Getattr((yyvsp[(4) - (5)].bases),"private"));
		   }
		   Setattr((yyval.node),"allows_typedef","1");

		   /* preserve the current scope */
		   Setattr((yyval.node),"prev_symtab",Swig_symbol_current());
		  
		   /* If the class name is qualified.  We need to create or lookup namespace/scope entries */
		   scope = resolve_create_node_scope((yyvsp[(3) - (5)].str));
		   /* save nscope_inner to the class - it may be overwritten in nested classes*/
		   Setattr((yyval.node), "nested:innerscope", nscope_inner);
		   Setattr((yyval.node), "nested:nscope", nscope);
		   Setfile(scope,cparse_file);
		   Setline(scope,cparse_line);
		   (yyvsp[(3) - (5)].str) = scope;
		   Setattr((yyval.node),"name",(yyvsp[(3) - (5)].str));

		   if (currentOuterClass) {
		     SetFlag((yyval.node), "nested");
		     Setattr((yyval.node), "nested:outer", currentOuterClass);
		     set_access_mode((yyval.node));
		   }
		   Swig_features_get(Swig_cparse_features(), Namespaceprefix, Getattr((yyval.node), "name"), 0, (yyval.node));
		   /* save yyrename to the class attribute, to be used later in add_symbols()*/
		   Setattr((yyval.node), "class_rename", make_name((yyval.node), (yyvsp[(3) - (5)].str), 0));
		   Setattr((yyval.node), "Classprefix", (yyvsp[(3) - (5)].str));
		   Classprefix = NewString((yyvsp[(3) - (5)].str));
		   /* Deal with inheritance  */
		   if ((yyvsp[(4) - (5)].bases))
		     bases = Swig_make_inherit_list((yyvsp[(3) - (5)].str),Getattr((yyvsp[(4) - (5)].bases),"public"),Namespaceprefix);
		   prefix = SwigType_istemplate_templateprefix((yyvsp[(3) - (5)].str));
		   if (prefix) {
		     String *fbase, *tbase;
		     if (Namespaceprefix) {
		       fbase = NewStringf("%s::%s", Namespaceprefix,(yyvsp[(3) - (5)].str));
		       tbase = NewStringf("%s::%s", Namespaceprefix, prefix);
		     } else {
		       fbase = Copy((yyvsp[(3) - (5)].str));
		       tbase = Copy(prefix);
		     }
		     Swig_name_inherit(tbase,fbase);
		     Delete(fbase);
		     Delete(tbase);
		   }
                   if (strcmp((yyvsp[(2) - (5)].id),"class") == 0) {
		     cplus_mode = CPLUS_PRIVATE;
		   } else {
		     cplus_mode = CPLUS_PUBLIC;
		   }
		   if (!cparse_cplusplus) {
		     set_scope_to_global();
		   }
		   Swig_symbol_newscope();
		   Swig_symbol_setscopename((yyvsp[(3) - (5)].str));
		   Swig_inherit_base_symbols(bases);
		   Delete(Namespaceprefix);
		   Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		   cparse_start_line = cparse_line;

		   /* If there are active template parameters, we need to make sure they are
                      placed in the class symbol table so we can catch shadows */

		   if (template_parameters) {
		     Parm *tp = template_parameters;
		     while(tp) {
		       String *tpname = Copy(Getattr(tp,"name"));
		       Node *tn = new_node("templateparm");
		       Setattr(tn,"name",tpname);
		       Swig_symbol_cadd(tpname,tn);
		       tp = nextSibling(tp);
		       Delete(tpname);
		     }
		   }
		   Delete(prefix);
		   inclass = 1;
		   currentOuterClass = (yyval.node);
		   if (cparse_cplusplusout) {
		     /* save the structure declaration to declare it in global scope for C++ to see */
		     code = get_raw_text_balanced('{', '}');
		     Setattr((yyval.node), "code", code);
		     Delete(code);
		   }
               }
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 3439 "parser.y"
    {
		   Node *p;
		   SwigType *ty;
		   Symtab *cscope;
		   Node *am = 0;
		   String *scpname = 0;
		   (void) (yyvsp[(6) - (9)].node);
		   (yyval.node) = currentOuterClass;
		   currentOuterClass = Getattr((yyval.node), "nested:outer");
		   nscope_inner = Getattr((yyval.node), "nested:innerscope");
		   nscope = Getattr((yyval.node), "nested:nscope");
		   Delattr((yyval.node), "nested:innerscope");
		   Delattr((yyval.node), "nested:nscope");
		   if (nscope_inner && Strcmp(nodeType(nscope_inner), "class") == 0) /* actual parent class for this class */
		     Setattr((yyval.node), "nested:outer", nscope_inner);
		   if (!currentOuterClass)
		     inclass = 0;
		   cscope = Getattr((yyval.node), "prev_symtab");
		   Delattr((yyval.node), "prev_symtab");
		   
		   /* Check for pure-abstract class */
		   Setattr((yyval.node),"abstracts", pure_abstracts((yyvsp[(7) - (9)].node)));
		   
		   /* This bit of code merges in a previously defined %extend directive (if any) */
		   {
		     String *clsname = Swig_symbol_qualifiedscopename(0);
		     am = Getattr(Swig_extend_hash(), clsname);
		     if (am) {
		       Swig_extend_merge((yyval.node), am);
		       Delattr(Swig_extend_hash(), clsname);
		     }
		     Delete(clsname);
		   }
		   if (!classes) classes = NewHash();
		   scpname = Swig_symbol_qualifiedscopename(0);
		   Setattr(classes, scpname, (yyval.node));

		   appendChild((yyval.node), (yyvsp[(7) - (9)].node));
		   
		   if (am) 
		     Swig_extend_append_previous((yyval.node), am);

		   p = (yyvsp[(9) - (9)].node);
		   if (p && !nscope_inner) {
		     if (!cparse_cplusplus && currentOuterClass)
		       appendChild(currentOuterClass, p);
		     else
		      appendSibling((yyval.node), p);
		   }
		   
		   if (nscope_inner) {
		     ty = NewString(scpname); /* if the class is declared out of scope, let the declarator use fully qualified type*/
		   } else if (cparse_cplusplus && !cparse_externc) {
		     ty = NewString((yyvsp[(3) - (9)].str));
		   } else {
		     ty = NewStringf("%s %s", (yyvsp[(2) - (9)].id), (yyvsp[(3) - (9)].str));
		   }
		   while (p) {
		     Setattr(p, "storage", (yyvsp[(1) - (9)].id));
		     Setattr(p, "type" ,ty);
		     if (!cparse_cplusplus && currentOuterClass && (!Getattr(currentOuterClass, "name"))) {
		       SetFlag(p, "hasconsttype");
		       SetFlag(p, "feature:immutable");
		     }
		     p = nextSibling(p);
		   }
		   if ((yyvsp[(9) - (9)].node) && Cmp((yyvsp[(1) - (9)].id),"typedef") == 0)
		     add_typedef_name((yyval.node), (yyvsp[(9) - (9)].node), (yyvsp[(3) - (9)].str), cscope, scpname);
		   Delete(scpname);

		   if (cplus_mode != CPLUS_PUBLIC) {
		   /* we 'open' the class at the end, to allow %template
		      to add new members */
		     Node *pa = new_node("access");
		     Setattr(pa, "kind", "public");
		     cplus_mode = CPLUS_PUBLIC;
		     appendChild((yyval.node), pa);
		     Delete(pa);
		   }
		   if (currentOuterClass)
		     restore_access_mode((yyval.node));
		   Setattr((yyval.node), "symtab", Swig_symbol_popscope());
		   Classprefix = Getattr((yyval.node), "Classprefix");
		   Delattr((yyval.node), "Classprefix");
		   Delete(Namespaceprefix);
		   Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		   if (cplus_mode == CPLUS_PRIVATE) {
		     (yyval.node) = 0; /* skip private nested classes */
		   } else if (cparse_cplusplus && currentOuterClass && ignore_nested_classes && !GetFlag((yyval.node), "feature:flatnested")) {
		     (yyval.node) = nested_forward_declaration((yyvsp[(1) - (9)].id), (yyvsp[(2) - (9)].id), (yyvsp[(3) - (9)].str), Copy((yyvsp[(3) - (9)].str)), (yyvsp[(9) - (9)].node));
		   } else if (nscope_inner) {
		     /* this is tricky */
		     /* we add the declaration in the original namespace */
		     appendChild(nscope_inner, (yyval.node));
		     Swig_symbol_setscope(Getattr(nscope_inner, "symtab"));
		     Delete(Namespaceprefix);
		     Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		     yyrename = Copy(Getattr((yyval.node), "class_rename"));
		     add_symbols((yyval.node));
		     Delattr((yyval.node), "class_rename");
		     /* but the variable definition in the current scope */
		     Swig_symbol_setscope(cscope);
		     Delete(Namespaceprefix);
		     Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		     add_symbols((yyvsp[(9) - (9)].node));
		     if (nscope) {
		       (yyval.node) = nscope; /* here we return recreated namespace tower instead of the class itself */
		       if ((yyvsp[(9) - (9)].node)) {
			 appendSibling((yyval.node), (yyvsp[(9) - (9)].node));
		       }
		     } else if (!SwigType_istemplate(ty) && template_parameters == 0) { /* for tempalte we need the class itself */
		       (yyval.node) = (yyvsp[(9) - (9)].node);
		     }
		   } else {
		     Delete(yyrename);
		     yyrename = 0;
		     if (!cparse_cplusplus && currentOuterClass) { /* nested C structs go into global scope*/
		       Node *outer = currentOuterClass;
		       while (Getattr(outer, "nested:outer"))
			 outer = Getattr(outer, "nested:outer");
		       appendSibling(outer, (yyval.node));
		       add_symbols((yyvsp[(9) - (9)].node));
		       set_scope_to_global();
		       Delete(Namespaceprefix);
		       Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		       yyrename = Copy(Getattr((yyval.node), "class_rename"));
		       add_symbols((yyval.node));
		       if (!cparse_cplusplusout)
			 Delattr((yyval.node), "nested:outer");
		       Delattr((yyval.node), "class_rename");
		       (yyval.node) = 0;
		     } else {
		       yyrename = Copy(Getattr((yyval.node), "class_rename"));
		       add_symbols((yyval.node));
		       add_symbols((yyvsp[(9) - (9)].node));
		       Delattr((yyval.node), "class_rename");
		     }
		   }
		   Delete(ty);
		   Swig_symbol_setscope(cscope);
		   Delete(Namespaceprefix);
		   Namespaceprefix = Swig_symbol_qualifiedscopename(0);
	       }
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 3585 "parser.y"
    {
	       String *unnamed;
	       String *code;
	       unnamed = make_unnamed();
	       (yyval.node) = new_node("class");
	       Setline((yyval.node),cparse_start_line);
	       Setattr((yyval.node),"kind",(yyvsp[(2) - (4)].id));
	       if ((yyvsp[(3) - (4)].bases)) {
		 Setattr((yyval.node),"baselist", Getattr((yyvsp[(3) - (4)].bases),"public"));
		 Setattr((yyval.node),"protectedbaselist", Getattr((yyvsp[(3) - (4)].bases),"protected"));
		 Setattr((yyval.node),"privatebaselist", Getattr((yyvsp[(3) - (4)].bases),"private"));
	       }
	       Setattr((yyval.node),"storage",(yyvsp[(1) - (4)].id));
	       Setattr((yyval.node),"unnamed",unnamed);
	       Setattr((yyval.node),"allows_typedef","1");
	       if (currentOuterClass) {
		 SetFlag((yyval.node), "nested");
		 Setattr((yyval.node), "nested:outer", currentOuterClass);
		 set_access_mode((yyval.node));
	       }
	       Swig_features_get(Swig_cparse_features(), Namespaceprefix, 0, 0, (yyval.node));
	       /* save yyrename to the class attribute, to be used later in add_symbols()*/
	       Setattr((yyval.node), "class_rename", make_name((yyval.node),0,0));
	       if (strcmp((yyvsp[(2) - (4)].id),"class") == 0) {
		 cplus_mode = CPLUS_PRIVATE;
	       } else {
		 cplus_mode = CPLUS_PUBLIC;
	       }
	       Swig_symbol_newscope();
	       cparse_start_line = cparse_line;
	       currentOuterClass = (yyval.node);
	       inclass = 1;
	       Classprefix = NewStringEmpty();
	       Delete(Namespaceprefix);
	       Namespaceprefix = Swig_symbol_qualifiedscopename(0);
	       /* save the structure declaration to make a typedef for it later*/
	       code = get_raw_text_balanced('{', '}');
	       Setattr((yyval.node), "code", code);
	       Delete(code);
	     }
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 3624 "parser.y"
    {
	       String *unnamed;
               List *bases = 0;
	       String *name = 0;
	       Node *n;
	       Classprefix = 0;
	       (yyval.node) = currentOuterClass;
	       currentOuterClass = Getattr((yyval.node), "nested:outer");
	       if (!currentOuterClass)
		 inclass = 0;
	       else
		 restore_access_mode((yyval.node));
	       unnamed = Getattr((yyval.node),"unnamed");
               /* Check for pure-abstract class */
	       Setattr((yyval.node),"abstracts", pure_abstracts((yyvsp[(6) - (8)].node)));
	       n = (yyvsp[(8) - (8)].node);
	       if (cparse_cplusplus && currentOuterClass && ignore_nested_classes && !GetFlag((yyval.node), "feature:flatnested")) {
		 String *name = n ? Copy(Getattr(n, "name")) : 0;
		 (yyval.node) = nested_forward_declaration((yyvsp[(1) - (8)].id), (yyvsp[(2) - (8)].id), 0, name, n);
		 Swig_symbol_popscope();
	         Delete(Namespaceprefix);
		 Namespaceprefix = Swig_symbol_qualifiedscopename(0);
	       } else if (n) {
	         appendSibling((yyval.node),n);
		 /* If a proper typedef name was given, we'll use it to set the scope name */
		 name = try_to_find_a_name_for_unnamed_structure((yyvsp[(1) - (8)].id), n);
		 if (name) {
		   String *scpname = 0;
		   SwigType *ty;
		   Setattr((yyval.node),"tdname",name);
		   Setattr((yyval.node),"name",name);
		   Swig_symbol_setscopename(name);
		   if ((yyvsp[(3) - (8)].bases))
		     bases = Swig_make_inherit_list(name,Getattr((yyvsp[(3) - (8)].bases),"public"),Namespaceprefix);
		   Swig_inherit_base_symbols(bases);

		     /* If a proper name was given, we use that as the typedef, not unnamed */
		   Clear(unnamed);
		   Append(unnamed, name);
		   if (cparse_cplusplus && !cparse_externc) {
		     ty = NewString(name);
		   } else {
		     ty = NewStringf("%s %s", (yyvsp[(2) - (8)].id),name);
		   }
		   while (n) {
		     Setattr(n,"storage",(yyvsp[(1) - (8)].id));
		     Setattr(n, "type", ty);
		     if (!cparse_cplusplus && currentOuterClass && (!Getattr(currentOuterClass, "name"))) {
		       SetFlag(n,"hasconsttype");
		       SetFlag(n,"feature:immutable");
		     }
		     n = nextSibling(n);
		   }
		   n = (yyvsp[(8) - (8)].node);

		   /* Check for previous extensions */
		   {
		     String *clsname = Swig_symbol_qualifiedscopename(0);
		     Node *am = Getattr(Swig_extend_hash(),clsname);
		     if (am) {
		       /* Merge the extension into the symbol table */
		       Swig_extend_merge((yyval.node),am);
		       Swig_extend_append_previous((yyval.node),am);
		       Delattr(Swig_extend_hash(),clsname);
		     }
		     Delete(clsname);
		   }
		   if (!classes) classes = NewHash();
		   scpname = Swig_symbol_qualifiedscopename(0);
		   Setattr(classes,scpname,(yyval.node));
		   Delete(scpname);
		 } else { /* no suitable name was found for a struct */
		   Setattr((yyval.node), "nested:unnamed", Getattr(n, "name")); /* save the name of the first declarator for later use in name generation*/
		   while (n) { /* attach unnamed struct to the declarators, so that they would receive proper type later*/
		     Setattr(n, "nested:unnamedtype", (yyval.node));
		     Setattr(n, "storage", (yyvsp[(1) - (8)].id));
		     n = nextSibling(n);
		   }
		   n = (yyvsp[(8) - (8)].node);
		   Swig_symbol_setscopename("<unnamed>");
		 }
		 appendChild((yyval.node),(yyvsp[(6) - (8)].node));
		 /* Pop the scope */
		 Setattr((yyval.node),"symtab",Swig_symbol_popscope());
		 if (name) {
		   Delete(yyrename);
		   yyrename = Copy(Getattr((yyval.node), "class_rename"));
		   Delete(Namespaceprefix);
		   Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		   add_symbols((yyval.node));
		   add_symbols(n);
		   Delattr((yyval.node), "class_rename");
		 }else if (cparse_cplusplus)
		   (yyval.node) = 0; /* ignore unnamed structs for C++ */
	         Delete(unnamed);
	       } else { /* unnamed struct w/o declarator*/
		 Swig_symbol_popscope();
	         Delete(Namespaceprefix);
		 Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		 add_symbols((yyvsp[(6) - (8)].node));
		 Delete((yyval.node));
		 (yyval.node) = (yyvsp[(6) - (8)].node); /* pass member list to outer class/namespace (instead of self)*/
	       }
              }
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 3730 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 3731 "parser.y"
    {
                        (yyval.node) = new_node("cdecl");
                        Setattr((yyval.node),"name",(yyvsp[(1) - (3)].decl).id);
                        Setattr((yyval.node),"decl",(yyvsp[(1) - (3)].decl).type);
                        Setattr((yyval.node),"parms",(yyvsp[(1) - (3)].decl).parms);
			set_nextSibling((yyval.node),(yyvsp[(3) - (3)].node));
                    }
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 3743 "parser.y"
    {
              if ((yyvsp[(1) - (4)].id) && (Strcmp((yyvsp[(1) - (4)].id),"friend") == 0)) {
		/* Ignore */
                (yyval.node) = 0; 
	      } else {
		(yyval.node) = new_node("classforward");
		Setattr((yyval.node),"kind",(yyvsp[(2) - (4)].id));
		Setattr((yyval.node),"name",(yyvsp[(3) - (4)].str));
		Setattr((yyval.node),"sym:weak", "1");
		add_symbols((yyval.node));
	      }
             }
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 3761 "parser.y"
    { 
		   if (currentOuterClass)
		     Setattr(currentOuterClass, "template_parameters", template_parameters);
		    template_parameters = (yyvsp[(3) - (4)].tparms); 
		  }
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 3765 "parser.y"
    {
			String *tname = 0;
			int     error = 0;

			/* check if we get a namespace node with a class declaration, and retrieve the class */
			Symtab *cscope = Swig_symbol_current();
			Symtab *sti = 0;
			Node *ntop = (yyvsp[(6) - (6)].node);
			Node *ni = ntop;
			SwigType *ntype = ni ? nodeType(ni) : 0;
			while (ni && Strcmp(ntype,"namespace") == 0) {
			  sti = Getattr(ni,"symtab");
			  ni = firstChild(ni);
			  ntype = nodeType(ni);
			}
			if (sti) {
			  Swig_symbol_setscope(sti);
			  Delete(Namespaceprefix);
			  Namespaceprefix = Swig_symbol_qualifiedscopename(0);
			  (yyvsp[(6) - (6)].node) = ni;
			}

			(yyval.node) = (yyvsp[(6) - (6)].node);
			if ((yyval.node)) tname = Getattr((yyval.node),"name");
			
			/* Check if the class is a template specialization */
			if (((yyval.node)) && (Strchr(tname,'<')) && (!is_operator(tname))) {
			  /* If a specialization.  Check if defined. */
			  Node *tempn = 0;
			  {
			    String *tbase = SwigType_templateprefix(tname);
			    tempn = Swig_symbol_clookup_local(tbase,0);
			    if (!tempn || (Strcmp(nodeType(tempn),"template") != 0)) {
			      SWIG_WARN_NODE_BEGIN(tempn);
			      Swig_warning(WARN_PARSE_TEMPLATE_SP_UNDEF, Getfile((yyval.node)),Getline((yyval.node)),"Specialization of non-template '%s'.\n", tbase);
			      SWIG_WARN_NODE_END(tempn);
			      tempn = 0;
			      error = 1;
			    }
			    Delete(tbase);
			  }
			  Setattr((yyval.node),"specialization","1");
			  Setattr((yyval.node),"templatetype",nodeType((yyval.node)));
			  set_nodeType((yyval.node),"template");
			  /* Template partial specialization */
			  if (tempn && ((yyvsp[(3) - (6)].tparms)) && ((yyvsp[(6) - (6)].node))) {
			    List   *tlist;
			    String *targs = SwigType_templateargs(tname);
			    tlist = SwigType_parmlist(targs);
			    /*			  Printf(stdout,"targs = '%s' %s\n", targs, tlist); */
			    if (!Getattr((yyval.node),"sym:weak")) {
			      Setattr((yyval.node),"sym:typename","1");
			    }
			    
			    if (Len(tlist) != ParmList_len(Getattr(tempn,"templateparms"))) {
			      Swig_error(Getfile((yyval.node)),Getline((yyval.node)),"Inconsistent argument count in template partial specialization. %d %d\n", Len(tlist), ParmList_len(Getattr(tempn,"templateparms")));
			      
			    } else {

			    /* This code builds the argument list for the partial template
			       specialization.  This is a little hairy, but the idea is as
			       follows:

			       $3 contains a list of arguments supplied for the template.
			       For example template<class T>.

			       tlist is a list of the specialization arguments--which may be
			       different.  For example class<int,T>.

			       tp is a copy of the arguments in the original template definition.
       
			       The patching algorithm walks through the list of supplied
			       arguments ($3), finds the position in the specialization arguments
			       (tlist), and then patches the name in the argument list of the
			       original template.
			    */

			    {
			      String *pn;
			      Parm *p, *p1;
			      int i, nargs;
			      Parm *tp = CopyParmList(Getattr(tempn,"templateparms"));
			      nargs = Len(tlist);
			      p = (yyvsp[(3) - (6)].tparms);
			      while (p) {
				for (i = 0; i < nargs; i++){
				  pn = Getattr(p,"name");
				  if (Strcmp(pn,SwigType_base(Getitem(tlist,i))) == 0) {
				    int j;
				    Parm *p1 = tp;
				    for (j = 0; j < i; j++) {
				      p1 = nextSibling(p1);
				    }
				    Setattr(p1,"name",pn);
				    Setattr(p1,"partialarg","1");
				  }
				}
				p = nextSibling(p);
			      }
			      p1 = tp;
			      i = 0;
			      while (p1) {
				if (!Getattr(p1,"partialarg")) {
				  Delattr(p1,"name");
				  Setattr(p1,"type", Getitem(tlist,i));
				} 
				i++;
				p1 = nextSibling(p1);
			      }
			      Setattr((yyval.node),"templateparms",tp);
			      Delete(tp);
			    }
  #if 0
			    /* Patch the parameter list */
			    if (tempn) {
			      Parm *p,*p1;
			      ParmList *tp = CopyParmList(Getattr(tempn,"templateparms"));
			      p = (yyvsp[(3) - (6)].tparms);
			      p1 = tp;
			      while (p && p1) {
				String *pn = Getattr(p,"name");
				Printf(stdout,"pn = '%s'\n", pn);
				if (pn) Setattr(p1,"name",pn);
				else Delattr(p1,"name");
				pn = Getattr(p,"type");
				if (pn) Setattr(p1,"type",pn);
				p = nextSibling(p);
				p1 = nextSibling(p1);
			      }
			      Setattr((yyval.node),"templateparms",tp);
			      Delete(tp);
			    } else {
			      Setattr((yyval.node),"templateparms",(yyvsp[(3) - (6)].tparms));
			    }
  #endif
			    Delattr((yyval.node),"specialization");
			    Setattr((yyval.node),"partialspecialization","1");
			    /* Create a specialized name for matching */
			    {
			      Parm *p = (yyvsp[(3) - (6)].tparms);
			      String *fname = NewString(Getattr((yyval.node),"name"));
			      String *ffname = 0;
			      ParmList *partialparms = 0;

			      char   tmp[32];
			      int    i, ilen;
			      while (p) {
				String *n = Getattr(p,"name");
				if (!n) {
				  p = nextSibling(p);
				  continue;
				}
				ilen = Len(tlist);
				for (i = 0; i < ilen; i++) {
				  if (Strstr(Getitem(tlist,i),n)) {
				    sprintf(tmp,"$%d",i+1);
				    Replaceid(fname,n,tmp);
				  }
				}
				p = nextSibling(p);
			      }
			      /* Patch argument names with typedef */
			      {
				Iterator tt;
				Parm *parm_current = 0;
				List *tparms = SwigType_parmlist(fname);
				ffname = SwigType_templateprefix(fname);
				Append(ffname,"<(");
				for (tt = First(tparms); tt.item; ) {
				  SwigType *rtt = Swig_symbol_typedef_reduce(tt.item,0);
				  SwigType *ttr = Swig_symbol_type_qualify(rtt,0);

				  Parm *newp = NewParmWithoutFileLineInfo(ttr, 0);
				  if (partialparms)
				    set_nextSibling(parm_current, newp);
				  else
				    partialparms = newp;
				  parm_current = newp;

				  Append(ffname,ttr);
				  tt = Next(tt);
				  if (tt.item) Putc(',',ffname);
				  Delete(rtt);
				  Delete(ttr);
				}
				Delete(tparms);
				Append(ffname,")>");
			      }
			      {
				Node *new_partial = NewHash();
				String *partials = Getattr(tempn,"partials");
				if (!partials) {
				  partials = NewList();
				  Setattr(tempn,"partials",partials);
				  Delete(partials);
				}
				/*			      Printf(stdout,"partial: fname = '%s', '%s'\n", fname, Swig_symbol_typedef_reduce(fname,0)); */
				Setattr(new_partial, "partialparms", partialparms);
				Setattr(new_partial, "templcsymname", ffname);
				Append(partials, new_partial);
			      }
			      Setattr((yyval.node),"partialargs",ffname);
			      Swig_symbol_cadd(ffname,(yyval.node));
			    }
			    }
			    Delete(tlist);
			    Delete(targs);
			  } else {
			    /* An explicit template specialization */
			    /* add default args from primary (unspecialized) template */
			    String *ty = Swig_symbol_template_deftype(tname,0);
			    String *fname = Swig_symbol_type_qualify(ty,0);
			    Swig_symbol_cadd(fname,(yyval.node));
			    Delete(ty);
			    Delete(fname);
			  }
			}  else if ((yyval.node)) {
			  Setattr((yyval.node),"templatetype",nodeType((yyvsp[(6) - (6)].node)));
			  set_nodeType((yyval.node),"template");
			  Setattr((yyval.node),"templateparms", (yyvsp[(3) - (6)].tparms));
			  if (!Getattr((yyval.node),"sym:weak")) {
			    Setattr((yyval.node),"sym:typename","1");
			  }
			  add_symbols((yyval.node));
			  default_arguments((yyval.node));
			  /* We also place a fully parameterized version in the symbol table */
			  {
			    Parm *p;
			    String *fname = NewStringf("%s<(", Getattr((yyval.node),"name"));
			    p = (yyvsp[(3) - (6)].tparms);
			    while (p) {
			      String *n = Getattr(p,"name");
			      if (!n) n = Getattr(p,"type");
			      Append(fname,n);
			      p = nextSibling(p);
			      if (p) Putc(',',fname);
			    }
			    Append(fname,")>");
			    Swig_symbol_cadd(fname,(yyval.node));
			  }
			}
			(yyval.node) = ntop;
			Swig_symbol_setscope(cscope);
			Delete(Namespaceprefix);
			Namespaceprefix = Swig_symbol_qualifiedscopename(0);
			if (error) (yyval.node) = 0;
			if (currentOuterClass)
			  template_parameters = Getattr(currentOuterClass, "template_parameters");
			else
			  template_parameters = 0;
                }
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 4018 "parser.y"
    {
		  Swig_warning(WARN_PARSE_EXPLICIT_TEMPLATE, cparse_file, cparse_line, "Explicit template instantiation ignored.\n");
                  (yyval.node) = 0; 
		}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 4024 "parser.y"
    {
		  Swig_warning(WARN_PARSE_EXPLICIT_TEMPLATE, cparse_file, cparse_line, "Explicit template instantiation ignored.\n");
                  (yyval.node) = 0; 
                }
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 4030 "parser.y"
    {
		  (yyval.node) = (yyvsp[(1) - (1)].node);
                }
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 4033 "parser.y"
    {
                   (yyval.node) = (yyvsp[(1) - (1)].node);
                }
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 4036 "parser.y"
    {
                   (yyval.node) = (yyvsp[(1) - (1)].node);
                }
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 4039 "parser.y"
    {
                   (yyval.node) = (yyvsp[(1) - (1)].node);
                }
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 4042 "parser.y"
    {
		  (yyval.node) = 0;
                }
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 4045 "parser.y"
    {
                  (yyval.node) = (yyvsp[(1) - (1)].node);
                }
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 4048 "parser.y"
    {
                  (yyval.node) = (yyvsp[(1) - (1)].node);
                }
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 4053 "parser.y"
    {
		   /* Rip out the parameter names */
		  Parm *p = (yyvsp[(1) - (1)].pl);
		  (yyval.tparms) = (yyvsp[(1) - (1)].pl);

		  while (p) {
		    String *name = Getattr(p,"name");
		    if (!name) {
		      /* Hmmm. Maybe it's a 'class T' parameter */
		      char *type = Char(Getattr(p,"type"));
		      /* Template template parameter */
		      if (strncmp(type,"template<class> ",16) == 0) {
			type += 16;
		      }
		      if ((strncmp(type,"class ",6) == 0) || (strncmp(type,"typename ", 9) == 0)) {
			char *t = strchr(type,' ');
			Setattr(p,"name", t+1);
		      } else 
                      /* Variadic template args */
		      if ((strncmp(type,"class... ",9) == 0) || (strncmp(type,"typename... ", 12) == 0)) {
			char *t = strchr(type,' ');
			Setattr(p,"name", t+1);
			Setattr(p,"variadic", "1");
		      } else {
			/*
			 Swig_error(cparse_file, cparse_line, "Missing template parameter name\n");
			 $$.rparms = 0;
			 $$.parms = 0;
			 break; */
		      }
		    }
		    p = nextSibling(p);
		  }
                 }
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 4089 "parser.y"
    {
                      set_nextSibling((yyvsp[(1) - (2)].p),(yyvsp[(2) - (2)].pl));
                      (yyval.pl) = (yyvsp[(1) - (2)].p);
                   }
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 4093 "parser.y"
    { (yyval.pl) = 0; }
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 4096 "parser.y"
    {
		    (yyval.p) = NewParmWithoutFileLineInfo(NewString((yyvsp[(1) - (1)].id)), 0);
                  }
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 4099 "parser.y"
    {
                    (yyval.p) = (yyvsp[(1) - (1)].p);
                  }
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 4104 "parser.y"
    {
                         set_nextSibling((yyvsp[(2) - (3)].p),(yyvsp[(3) - (3)].pl));
                         (yyval.pl) = (yyvsp[(2) - (3)].p);
                       }
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 4108 "parser.y"
    { (yyval.pl) = 0; }
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 4113 "parser.y"
    {
                  String *uname = Swig_symbol_type_qualify((yyvsp[(2) - (3)].str),0);
		  String *name = Swig_scopename_last((yyvsp[(2) - (3)].str));
                  (yyval.node) = new_node("using");
		  Setattr((yyval.node),"uname",uname);
		  Setattr((yyval.node),"name", name);
		  Delete(uname);
		  Delete(name);
		  add_symbols((yyval.node));
             }
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 4123 "parser.y"
    {
	       Node *n = Swig_symbol_clookup((yyvsp[(3) - (4)].str),0);
	       if (!n) {
		 Swig_error(cparse_file, cparse_line, "Nothing known about namespace '%s'\n", (yyvsp[(3) - (4)].str));
		 (yyval.node) = 0;
	       } else {

		 while (Strcmp(nodeType(n),"using") == 0) {
		   n = Getattr(n,"node");
		 }
		 if (n) {
		   if (Strcmp(nodeType(n),"namespace") == 0) {
		     Symtab *current = Swig_symbol_current();
		     Symtab *symtab = Getattr(n,"symtab");
		     (yyval.node) = new_node("using");
		     Setattr((yyval.node),"node",n);
		     Setattr((yyval.node),"namespace", (yyvsp[(3) - (4)].str));
		     if (current != symtab) {
		       Swig_symbol_inherit(symtab);
		     }
		   } else {
		     Swig_error(cparse_file, cparse_line, "'%s' is not a namespace.\n", (yyvsp[(3) - (4)].str));
		     (yyval.node) = 0;
		   }
		 } else {
		   (yyval.node) = 0;
		 }
	       }
             }
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 4154 "parser.y"
    { 
                Hash *h;
                (yyvsp[(1) - (3)].node) = Swig_symbol_current();
		h = Swig_symbol_clookup((yyvsp[(2) - (3)].str),0);
		if (h && ((yyvsp[(1) - (3)].node) == Getattr(h,"sym:symtab")) && (Strcmp(nodeType(h),"namespace") == 0)) {
		  if (Getattr(h,"alias")) {
		    h = Getattr(h,"namespace");
		    Swig_warning(WARN_PARSE_NAMESPACE_ALIAS, cparse_file, cparse_line, "Namespace alias '%s' not allowed here. Assuming '%s'\n",
				 (yyvsp[(2) - (3)].str), Getattr(h,"name"));
		    (yyvsp[(2) - (3)].str) = Getattr(h,"name");
		  }
		  Swig_symbol_setscope(Getattr(h,"symtab"));
		} else {
		  Swig_symbol_newscope();
		  Swig_symbol_setscopename((yyvsp[(2) - (3)].str));
		}
		Delete(Namespaceprefix);
		Namespaceprefix = Swig_symbol_qualifiedscopename(0);
             }
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 4172 "parser.y"
    {
                Node *n = (yyvsp[(5) - (6)].node);
		set_nodeType(n,"namespace");
		Setattr(n,"name",(yyvsp[(2) - (6)].str));
                Setattr(n,"symtab", Swig_symbol_popscope());
		Swig_symbol_setscope((yyvsp[(1) - (6)].node));
		(yyval.node) = n;
		Delete(Namespaceprefix);
		Namespaceprefix = Swig_symbol_qualifiedscopename(0);
		add_symbols((yyval.node));
             }
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 4183 "parser.y"
    {
	       Hash *h;
	       (yyvsp[(1) - (2)].node) = Swig_symbol_current();
	       h = Swig_symbol_clookup("    ",0);
	       if (h && (Strcmp(nodeType(h),"namespace") == 0)) {
		 Swig_symbol_setscope(Getattr(h,"symtab"));
	       } else {
		 Swig_symbol_newscope();
		 /* we don't use "__unnamed__", but a long 'empty' name */
		 Swig_symbol_setscopename("    ");
	       }
	       Namespaceprefix = 0;
             }
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 4195 "parser.y"
    {
	       (yyval.node) = (yyvsp[(4) - (5)].node);
	       set_nodeType((yyval.node),"namespace");
	       Setattr((yyval.node),"unnamed","1");
	       Setattr((yyval.node),"symtab", Swig_symbol_popscope());
	       Swig_symbol_setscope((yyvsp[(1) - (5)].node));
	       Delete(Namespaceprefix);
	       Namespaceprefix = Swig_symbol_qualifiedscopename(0);
	       add_symbols((yyval.node));
             }
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 4205 "parser.y"
    {
	       /* Namespace alias */
	       Node *n;
	       (yyval.node) = new_node("namespace");
	       Setattr((yyval.node),"name",(yyvsp[(2) - (5)].id));
	       Setattr((yyval.node),"alias",(yyvsp[(4) - (5)].str));
	       n = Swig_symbol_clookup((yyvsp[(4) - (5)].str),0);
	       if (!n) {
		 Swig_error(cparse_file, cparse_line, "Unknown namespace '%s'\n", (yyvsp[(4) - (5)].str));
		 (yyval.node) = 0;
	       } else {
		 if (Strcmp(nodeType(n),"namespace") != 0) {
		   Swig_error(cparse_file, cparse_line, "'%s' is not a namespace\n",(yyvsp[(4) - (5)].str));
		   (yyval.node) = 0;
		 } else {
		   while (Getattr(n,"alias")) {
		     n = Getattr(n,"namespace");
		   }
		   Setattr((yyval.node),"namespace",n);
		   add_symbols((yyval.node));
		   /* Set up a scope alias */
		   Swig_symbol_alias((yyvsp[(2) - (5)].id),Getattr(n,"symtab"));
		 }
	       }
             }
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 4232 "parser.y"
    {
                   (yyval.node) = (yyvsp[(1) - (2)].node);
                   /* Insert cpp_member (including any siblings) to the front of the cpp_members linked list */
		   if ((yyval.node)) {
		     Node *p = (yyval.node);
		     Node *pp =0;
		     while (p) {
		       pp = p;
		       p = nextSibling(p);
		     }
		     set_nextSibling(pp,(yyvsp[(2) - (2)].node));
		     if ((yyvsp[(2) - (2)].node))
		       set_previousSibling((yyvsp[(2) - (2)].node), pp);
		   } else {
		     (yyval.node) = (yyvsp[(2) - (2)].node);
		   }
             }
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 4249 "parser.y"
    { 
	       extendmode = 1;
	       if (cplus_mode != CPLUS_PUBLIC) {
		 Swig_error(cparse_file,cparse_line,"%%extend can only be used in a public section\n");
	       }
             }
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 4254 "parser.y"
    {
	       extendmode = 0;
	     }
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 4256 "parser.y"
    {
	       (yyval.node) = new_node("extend");
	       mark_nodes_as_extend((yyvsp[(4) - (7)].node));
	       appendChild((yyval.node),(yyvsp[(4) - (7)].node));
	       set_nextSibling((yyval.node),(yyvsp[(7) - (7)].node));
	     }
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 4262 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 4263 "parser.y"
    { (yyval.node) = 0;}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 4264 "parser.y"
    {
	       int start_line = cparse_line;
	       skip_decl();
	       Swig_error(cparse_file,start_line,"Syntax error in input(3).\n");
	       exit(1);
	       }
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 4269 "parser.y"
    { 
		 (yyval.node) = (yyvsp[(3) - (3)].node);
   	     }
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 4280 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 4281 "parser.y"
    { 
                 (yyval.node) = (yyvsp[(1) - (1)].node); 
		 if (extendmode && current_class) {
		   String *symname;
		   symname= make_name((yyval.node),Getattr((yyval.node),"name"), Getattr((yyval.node),"decl"));
		   if (Strcmp(symname,Getattr((yyval.node),"name")) == 0) {
		     /* No renaming operation.  Set name to class name */
		     Delete(yyrename);
		     yyrename = NewString(Getattr(current_class,"sym:name"));
		   } else {
		     Delete(yyrename);
		     yyrename = symname;
		   }
		 }
		 add_symbols((yyval.node));
                 default_arguments((yyval.node));
             }
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 4298 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 4299 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 4300 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 4301 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 4302 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 4303 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 4304 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 4305 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 4306 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 4307 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 4308 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 4309 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 4310 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 4311 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 4312 "parser.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 4313 "parser.y"
    {(yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 4314 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 4323 "parser.y"
    {
              if (inclass || extendmode) {
		SwigType *decl = NewStringEmpty();
		(yyval.node) = new_node("constructor");
		Setattr((yyval.node),"storage",(yyvsp[(1) - (6)].id));
		Setattr((yyval.node),"name",(yyvsp[(2) - (6)].type));
		Setattr((yyval.node),"parms",(yyvsp[(4) - (6)].pl));
		SwigType_add_function(decl,(yyvsp[(4) - (6)].pl));
		Setattr((yyval.node),"decl",decl);
		Setattr((yyval.node),"throws",(yyvsp[(6) - (6)].decl).throws);
		Setattr((yyval.node),"throw",(yyvsp[(6) - (6)].decl).throwf);
		Setattr((yyval.node),"noexcept",(yyvsp[(6) - (6)].decl).nexcept);
		if (Len(scanner_ccode)) {
		  String *code = Copy(scanner_ccode);
		  Setattr((yyval.node),"code",code);
		  Delete(code);
		}
		SetFlag((yyval.node),"feature:new");
		if ((yyvsp[(6) - (6)].decl).defarg)
		  Setattr((yyval.node),"value",(yyvsp[(6) - (6)].decl).defarg);
	      } else {
		(yyval.node) = 0;
              }
              }
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 4351 "parser.y"
    {
               String *name = NewStringf("%s",(yyvsp[(2) - (6)].str));
	       if (*(Char(name)) != '~') Insert(name,0,"~");
               (yyval.node) = new_node("destructor");
	       Setattr((yyval.node),"name",name);
	       Delete(name);
	       if (Len(scanner_ccode)) {
		 String *code = Copy(scanner_ccode);
		 Setattr((yyval.node),"code",code);
		 Delete(code);
	       }
	       {
		 String *decl = NewStringEmpty();
		 SwigType_add_function(decl,(yyvsp[(4) - (6)].pl));
		 Setattr((yyval.node),"decl",decl);
		 Delete(decl);
	       }
	       Setattr((yyval.node),"throws",(yyvsp[(6) - (6)].dtype).throws);
	       Setattr((yyval.node),"throw",(yyvsp[(6) - (6)].dtype).throwf);
	       Setattr((yyval.node),"noexcept",(yyvsp[(6) - (6)].dtype).nexcept);
	       if ((yyvsp[(6) - (6)].dtype).val)
	         Setattr((yyval.node),"value",(yyvsp[(6) - (6)].dtype).val);
	       add_symbols((yyval.node));
	      }
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 4378 "parser.y"
    {
		String *name;
		(yyval.node) = new_node("destructor");
		Setattr((yyval.node),"storage","virtual");
	        name = NewStringf("%s",(yyvsp[(3) - (7)].str));
		if (*(Char(name)) != '~') Insert(name,0,"~");
		Setattr((yyval.node),"name",name);
		Delete(name);
		Setattr((yyval.node),"throws",(yyvsp[(7) - (7)].dtype).throws);
		Setattr((yyval.node),"throw",(yyvsp[(7) - (7)].dtype).throwf);
		Setattr((yyval.node),"noexcept",(yyvsp[(7) - (7)].dtype).nexcept);
		if ((yyvsp[(7) - (7)].dtype).val)
		  Setattr((yyval.node),"value",(yyvsp[(7) - (7)].dtype).val);
		if (Len(scanner_ccode)) {
		  String *code = Copy(scanner_ccode);
		  Setattr((yyval.node),"code",code);
		  Delete(code);
		}
		{
		  String *decl = NewStringEmpty();
		  SwigType_add_function(decl,(yyvsp[(5) - (7)].pl));
		  Setattr((yyval.node),"decl",decl);
		  Delete(decl);
		}

		add_symbols((yyval.node));
	      }
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 4409 "parser.y"
    {
                 (yyval.node) = new_node("cdecl");
                 Setattr((yyval.node),"type",(yyvsp[(3) - (8)].type));
		 Setattr((yyval.node),"name",(yyvsp[(2) - (8)].str));
		 Setattr((yyval.node),"storage",(yyvsp[(1) - (8)].id));

		 SwigType_add_function((yyvsp[(4) - (8)].type),(yyvsp[(6) - (8)].pl));
		 if ((yyvsp[(8) - (8)].dtype).qualifier) {
		   SwigType_push((yyvsp[(4) - (8)].type),(yyvsp[(8) - (8)].dtype).qualifier);
		 }
		 Setattr((yyval.node),"decl",(yyvsp[(4) - (8)].type));
		 Setattr((yyval.node),"parms",(yyvsp[(6) - (8)].pl));
		 Setattr((yyval.node),"conversion_operator","1");
		 add_symbols((yyval.node));
              }
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 4424 "parser.y"
    {
		 SwigType *decl;
                 (yyval.node) = new_node("cdecl");
                 Setattr((yyval.node),"type",(yyvsp[(3) - (8)].type));
		 Setattr((yyval.node),"name",(yyvsp[(2) - (8)].str));
		 Setattr((yyval.node),"storage",(yyvsp[(1) - (8)].id));
		 decl = NewStringEmpty();
		 SwigType_add_reference(decl);
		 SwigType_add_function(decl,(yyvsp[(6) - (8)].pl));
		 if ((yyvsp[(8) - (8)].dtype).qualifier) {
		   SwigType_push(decl,(yyvsp[(8) - (8)].dtype).qualifier);
		 }
		 Setattr((yyval.node),"decl",decl);
		 Setattr((yyval.node),"parms",(yyvsp[(6) - (8)].pl));
		 Setattr((yyval.node),"conversion_operator","1");
		 add_symbols((yyval.node));
	       }
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 4441 "parser.y"
    {
		 SwigType *decl;
                 (yyval.node) = new_node("cdecl");
                 Setattr((yyval.node),"type",(yyvsp[(3) - (8)].type));
		 Setattr((yyval.node),"name",(yyvsp[(2) - (8)].str));
		 Setattr((yyval.node),"storage",(yyvsp[(1) - (8)].id));
		 decl = NewStringEmpty();
		 SwigType_add_rvalue_reference(decl);
		 SwigType_add_function(decl,(yyvsp[(6) - (8)].pl));
		 if ((yyvsp[(8) - (8)].dtype).qualifier) {
		   SwigType_push(decl,(yyvsp[(8) - (8)].dtype).qualifier);
		 }
		 Setattr((yyval.node),"decl",decl);
		 Setattr((yyval.node),"parms",(yyvsp[(6) - (8)].pl));
		 Setattr((yyval.node),"conversion_operator","1");
		 add_symbols((yyval.node));
	       }
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 4459 "parser.y"
    {
		 SwigType *decl;
                 (yyval.node) = new_node("cdecl");
                 Setattr((yyval.node),"type",(yyvsp[(3) - (9)].type));
		 Setattr((yyval.node),"name",(yyvsp[(2) - (9)].str));
		 Setattr((yyval.node),"storage",(yyvsp[(1) - (9)].id));
		 decl = NewStringEmpty();
		 SwigType_add_pointer(decl);
		 SwigType_add_reference(decl);
		 SwigType_add_function(decl,(yyvsp[(7) - (9)].pl));
		 if ((yyvsp[(9) - (9)].dtype).qualifier) {
		   SwigType_push(decl,(yyvsp[(9) - (9)].dtype).qualifier);
		 }
		 Setattr((yyval.node),"decl",decl);
		 Setattr((yyval.node),"parms",(yyvsp[(7) - (9)].pl));
		 Setattr((yyval.node),"conversion_operator","1");
		 add_symbols((yyval.node));
	       }
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 4478 "parser.y"
    {
		String *t = NewStringEmpty();
		(yyval.node) = new_node("cdecl");
		Setattr((yyval.node),"type",(yyvsp[(3) - (7)].type));
		Setattr((yyval.node),"name",(yyvsp[(2) - (7)].str));
		 Setattr((yyval.node),"storage",(yyvsp[(1) - (7)].id));
		SwigType_add_function(t,(yyvsp[(5) - (7)].pl));
		if ((yyvsp[(7) - (7)].dtype).qualifier) {
		  SwigType_push(t,(yyvsp[(7) - (7)].dtype).qualifier);
		}
		Setattr((yyval.node),"decl",t);
		Setattr((yyval.node),"parms",(yyvsp[(5) - (7)].pl));
		Setattr((yyval.node),"conversion_operator","1");
		add_symbols((yyval.node));
              }
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 4497 "parser.y"
    {
                 skip_balanced('{','}');
                 (yyval.node) = 0;
               }
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 4504 "parser.y"
    {
                skip_balanced('(',')');
                (yyval.node) = 0;
              }
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 4511 "parser.y"
    { 
                (yyval.node) = new_node("access");
		Setattr((yyval.node),"kind","public");
                cplus_mode = CPLUS_PUBLIC;
              }
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 4518 "parser.y"
    { 
                (yyval.node) = new_node("access");
                Setattr((yyval.node),"kind","private");
		cplus_mode = CPLUS_PRIVATE;
	      }
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 4526 "parser.y"
    { 
		(yyval.node) = new_node("access");
		Setattr((yyval.node),"kind","protected");
		cplus_mode = CPLUS_PROTECTED;
	      }
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 4534 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 4537 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 4541 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 4544 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 4545 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 4546 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 4547 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 4548 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 4549 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 4550 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 4551 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 4554 "parser.y"
    {
	            Clear(scanner_ccode);
		    (yyval.dtype).val = 0;
		    (yyval.dtype).throws = (yyvsp[(1) - (2)].dtype).throws;
		    (yyval.dtype).throwf = (yyvsp[(1) - (2)].dtype).throwf;
		    (yyval.dtype).nexcept = (yyvsp[(1) - (2)].dtype).nexcept;
               }
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 4561 "parser.y"
    {
	            Clear(scanner_ccode);
		    (yyval.dtype).val = (yyvsp[(3) - (4)].dtype).val;
		    (yyval.dtype).throws = (yyvsp[(1) - (4)].dtype).throws;
		    (yyval.dtype).throwf = (yyvsp[(1) - (4)].dtype).throwf;
		    (yyval.dtype).nexcept = (yyvsp[(1) - (4)].dtype).nexcept;
               }
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 4568 "parser.y"
    { 
		    skip_balanced('{','}'); 
		    (yyval.dtype).val = 0;
		    (yyval.dtype).throws = (yyvsp[(1) - (2)].dtype).throws;
		    (yyval.dtype).throwf = (yyvsp[(1) - (2)].dtype).throwf;
		    (yyval.dtype).nexcept = (yyvsp[(1) - (2)].dtype).nexcept;
	       }
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 4577 "parser.y"
    { 
                     Clear(scanner_ccode);
                     (yyval.dtype).val = 0;
                     (yyval.dtype).qualifier = (yyvsp[(1) - (2)].dtype).qualifier;
                     (yyval.dtype).bitfield = 0;
                     (yyval.dtype).throws = (yyvsp[(1) - (2)].dtype).throws;
                     (yyval.dtype).throwf = (yyvsp[(1) - (2)].dtype).throwf;
                     (yyval.dtype).nexcept = (yyvsp[(1) - (2)].dtype).nexcept;
                }
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 4586 "parser.y"
    { 
                     Clear(scanner_ccode);
                     (yyval.dtype).val = (yyvsp[(3) - (4)].dtype).val;
                     (yyval.dtype).qualifier = (yyvsp[(1) - (4)].dtype).qualifier;
                     (yyval.dtype).bitfield = 0;
                     (yyval.dtype).throws = (yyvsp[(1) - (4)].dtype).throws; 
                     (yyval.dtype).throwf = (yyvsp[(1) - (4)].dtype).throwf; 
                     (yyval.dtype).nexcept = (yyvsp[(1) - (4)].dtype).nexcept; 
               }
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 4595 "parser.y"
    { 
                     skip_balanced('{','}');
                     (yyval.dtype).val = 0;
                     (yyval.dtype).qualifier = (yyvsp[(1) - (2)].dtype).qualifier;
                     (yyval.dtype).bitfield = 0;
                     (yyval.dtype).throws = (yyvsp[(1) - (2)].dtype).throws; 
                     (yyval.dtype).throwf = (yyvsp[(1) - (2)].dtype).throwf; 
                     (yyval.dtype).nexcept = (yyvsp[(1) - (2)].dtype).nexcept; 
               }
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 4607 "parser.y"
    { }
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 4610 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type);
                  /* Printf(stdout,"primitive = '%s'\n", $$);*/
                }
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 4613 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type); }
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 4614 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type); }
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 4615 "parser.y"
    { (yyval.type) = NewStringf("%s%s",(yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].id)); }
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 4616 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type); }
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 4618 "parser.y"
    {
		  (yyval.type) = (yyvsp[(1) - (1)].str);
               }
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 4627 "parser.y"
    { (yyval.id) = "extern"; }
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 4628 "parser.y"
    {
                   if (strcmp((yyvsp[(2) - (2)].id),"C") == 0) {
		     (yyval.id) = "externc";
                   } else if (strcmp((yyvsp[(2) - (2)].id),"C++") == 0) {
		     (yyval.id) = "extern";
		   } else {
		     Swig_warning(WARN_PARSE_UNDEFINED_EXTERN,cparse_file, cparse_line,"Unrecognized extern type \"%s\".\n", (yyvsp[(2) - (2)].id));
		     (yyval.id) = 0;
		   }
               }
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 4638 "parser.y"
    {
                   if (strcmp((yyvsp[(2) - (3)].id),"C") == 0) {
		     (yyval.id) = "externc thread_local";
                   } else if (strcmp((yyvsp[(2) - (3)].id),"C++") == 0) {
		     (yyval.id) = "extern thread_local";
		   } else {
		     Swig_warning(WARN_PARSE_UNDEFINED_EXTERN,cparse_file, cparse_line,"Unrecognized extern type \"%s\".\n", (yyvsp[(2) - (3)].id));
		     (yyval.id) = 0;
		   }
               }
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 4648 "parser.y"
    { (yyval.id) = "static"; }
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 4649 "parser.y"
    { (yyval.id) = "typedef"; }
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 4650 "parser.y"
    { (yyval.id) = "virtual"; }
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 4651 "parser.y"
    { (yyval.id) = "friend"; }
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 4652 "parser.y"
    { (yyval.id) = "explicit"; }
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 4653 "parser.y"
    { (yyval.id) = "constexpr"; }
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 4654 "parser.y"
    { (yyval.id) = "static constexpr"; }
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 4655 "parser.y"
    { (yyval.id) = "thread_local"; }
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 4656 "parser.y"
    { (yyval.id) = "static thread_local"; }
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 4657 "parser.y"
    { (yyval.id) = "static thread_local"; }
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 4658 "parser.y"
    { (yyval.id) = "extern thread_local"; }
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 4659 "parser.y"
    { (yyval.id) = "extern thread_local"; }
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 4660 "parser.y"
    { (yyval.id) = 0; }
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 4667 "parser.y"
    {
                 Parm *p;
		 (yyval.pl) = (yyvsp[(1) - (1)].pl);
		 p = (yyvsp[(1) - (1)].pl);
                 while (p) {
		   Replace(Getattr(p,"type"),"typename ", "", DOH_REPLACE_ANY);
		   p = nextSibling(p);
                 }
               }
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 4678 "parser.y"
    {
                  set_nextSibling((yyvsp[(1) - (2)].p),(yyvsp[(2) - (2)].pl));
                  (yyval.pl) = (yyvsp[(1) - (2)].p);
		}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 4682 "parser.y"
    { (yyval.pl) = 0; }
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 4685 "parser.y"
    {
                 set_nextSibling((yyvsp[(2) - (3)].p),(yyvsp[(3) - (3)].pl));
		 (yyval.pl) = (yyvsp[(2) - (3)].p);
                }
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 4689 "parser.y"
    { (yyval.pl) = 0; }
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 4693 "parser.y"
    {
                   SwigType_push((yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].decl).type);
		   (yyval.p) = NewParmWithoutFileLineInfo((yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].decl).id);
		   Setfile((yyval.p),cparse_file);
		   Setline((yyval.p),cparse_line);
		   if ((yyvsp[(2) - (2)].decl).defarg) {
		     Setattr((yyval.p),"value",(yyvsp[(2) - (2)].decl).defarg);
		   }
		}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 4703 "parser.y"
    {
                  (yyval.p) = NewParmWithoutFileLineInfo(NewStringf("template<class> %s %s", (yyvsp[(5) - (7)].id),(yyvsp[(6) - (7)].str)), 0);
		  Setfile((yyval.p),cparse_file);
		  Setline((yyval.p),cparse_line);
                  if ((yyvsp[(7) - (7)].dtype).val) {
                    Setattr((yyval.p),"value",(yyvsp[(7) - (7)].dtype).val);
                  }
                }
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 4711 "parser.y"
    {
		  SwigType *t = NewString("v(...)");
		  (yyval.p) = NewParmWithoutFileLineInfo(t, 0);
		  Setfile((yyval.p),cparse_file);
		  Setline((yyval.p),cparse_line);
		}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 4719 "parser.y"
    {
                 Parm *p;
		 (yyval.p) = (yyvsp[(1) - (1)].p);
		 p = (yyvsp[(1) - (1)].p);
                 while (p) {
		   if (Getattr(p,"type")) {
		     Replace(Getattr(p,"type"),"typename ", "", DOH_REPLACE_ANY);
		   }
		   p = nextSibling(p);
                 }
               }
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 4732 "parser.y"
    {
                  set_nextSibling((yyvsp[(1) - (2)].p),(yyvsp[(2) - (2)].p));
                  (yyval.p) = (yyvsp[(1) - (2)].p);
		}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 4736 "parser.y"
    { (yyval.p) = 0; }
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 4739 "parser.y"
    {
                 set_nextSibling((yyvsp[(2) - (3)].p),(yyvsp[(3) - (3)].p));
		 (yyval.p) = (yyvsp[(2) - (3)].p);
                }
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 4743 "parser.y"
    { (yyval.p) = 0; }
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 4747 "parser.y"
    {
		  (yyval.p) = (yyvsp[(1) - (1)].p);
		  {
		    /* We need to make a possible adjustment for integer parameters. */
		    SwigType *type;
		    Node     *n = 0;

		    while (!n) {
		      type = Getattr((yyvsp[(1) - (1)].p),"type");
		      n = Swig_symbol_clookup(type,0);     /* See if we can find a node that matches the typename */
		      if ((n) && (Strcmp(nodeType(n),"cdecl") == 0)) {
			SwigType *decl = Getattr(n,"decl");
			if (!SwigType_isfunction(decl)) {
			  String *value = Getattr(n,"value");
			  if (value) {
			    String *v = Copy(value);
			    Setattr((yyvsp[(1) - (1)].p),"type",v);
			    Delete(v);
			    n = 0;
			  }
			}
		      } else {
			break;
		      }
		    }
		  }

               }
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 4775 "parser.y"
    {
                  (yyval.p) = NewParmWithoutFileLineInfo(0,0);
                  Setfile((yyval.p),cparse_file);
		  Setline((yyval.p),cparse_line);
		  Setattr((yyval.p),"value",(yyvsp[(1) - (1)].dtype).val);
               }
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 4783 "parser.y"
    { 
                  (yyval.dtype) = (yyvsp[(2) - (2)].dtype); 
		  if ((yyvsp[(2) - (2)].dtype).type == T_ERROR) {
		    Swig_warning(WARN_PARSE_BAD_DEFAULT,cparse_file, cparse_line, "Can't set default argument (ignored)\n");
		    (yyval.dtype).val = 0;
		    (yyval.dtype).rawval = 0;
		    (yyval.dtype).bitfield = 0;
		    (yyval.dtype).throws = 0;
		    (yyval.dtype).throwf = 0;
		    (yyval.dtype).nexcept = 0;
		  }
               }
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 4795 "parser.y"
    { 
		  (yyval.dtype) = (yyvsp[(2) - (5)].dtype);
		  if ((yyvsp[(2) - (5)].dtype).type == T_ERROR) {
		    Swig_warning(WARN_PARSE_BAD_DEFAULT,cparse_file, cparse_line, "Can't set default argument (ignored)\n");
		    (yyval.dtype) = (yyvsp[(2) - (5)].dtype);
		    (yyval.dtype).val = 0;
		    (yyval.dtype).rawval = 0;
		    (yyval.dtype).bitfield = 0;
		    (yyval.dtype).throws = 0;
		    (yyval.dtype).throwf = 0;
		    (yyval.dtype).nexcept = 0;
		  } else {
		    (yyval.dtype).val = NewStringf("%s[%s]",(yyvsp[(2) - (5)].dtype).val,(yyvsp[(4) - (5)].dtype).val); 
		  }		  
               }
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 4810 "parser.y"
    {
		 skip_balanced('{','}');
		 (yyval.dtype).val = NewString(scanner_ccode);
		 (yyval.dtype).rawval = 0;
                 (yyval.dtype).type = T_INT;
		 (yyval.dtype).bitfield = 0;
		 (yyval.dtype).throws = 0;
		 (yyval.dtype).throwf = 0;
		 (yyval.dtype).nexcept = 0;
	       }
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 4820 "parser.y"
    { 
		 (yyval.dtype).val = 0;
		 (yyval.dtype).rawval = 0;
		 (yyval.dtype).type = 0;
		 (yyval.dtype).bitfield = (yyvsp[(2) - (2)].dtype).val;
		 (yyval.dtype).throws = 0;
		 (yyval.dtype).throwf = 0;
		 (yyval.dtype).nexcept = 0;
	       }
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 4829 "parser.y"
    {
                 (yyval.dtype).val = 0;
                 (yyval.dtype).rawval = 0;
                 (yyval.dtype).type = T_INT;
		 (yyval.dtype).bitfield = 0;
		 (yyval.dtype).throws = 0;
		 (yyval.dtype).throwf = 0;
		 (yyval.dtype).nexcept = 0;
               }
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 4840 "parser.y"
    {
                 (yyval.decl) = (yyvsp[(1) - (2)].decl);
		 (yyval.decl).defarg = (yyvsp[(2) - (2)].dtype).rawval ? (yyvsp[(2) - (2)].dtype).rawval : (yyvsp[(2) - (2)].dtype).val;
            }
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 4844 "parser.y"
    {
              (yyval.decl) = (yyvsp[(1) - (2)].decl);
	      (yyval.decl).defarg = (yyvsp[(2) - (2)].dtype).rawval ? (yyvsp[(2) - (2)].dtype).rawval : (yyvsp[(2) - (2)].dtype).val;
            }
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 4848 "parser.y"
    {
   	      (yyval.decl).type = 0;
              (yyval.decl).id = 0;
	      (yyval.decl).defarg = (yyvsp[(1) - (1)].dtype).rawval ? (yyvsp[(1) - (1)].dtype).rawval : (yyvsp[(1) - (1)].dtype).val;
            }
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 4855 "parser.y"
    {
                 (yyval.decl) = (yyvsp[(1) - (1)].decl);
		 if (SwigType_isfunction((yyvsp[(1) - (1)].decl).type)) {
		   Delete(SwigType_pop_function((yyvsp[(1) - (1)].decl).type));
		 } else if (SwigType_isarray((yyvsp[(1) - (1)].decl).type)) {
		   SwigType *ta = SwigType_pop_arrays((yyvsp[(1) - (1)].decl).type);
		   if (SwigType_isfunction((yyvsp[(1) - (1)].decl).type)) {
		     Delete(SwigType_pop_function((yyvsp[(1) - (1)].decl).type));
		   } else {
		     (yyval.decl).parms = 0;
		   }
		   SwigType_push((yyvsp[(1) - (1)].decl).type,ta);
		   Delete(ta);
		 } else {
		   (yyval.decl).parms = 0;
		 }
            }
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 4872 "parser.y"
    {
              (yyval.decl) = (yyvsp[(1) - (1)].decl);
	      if (SwigType_isfunction((yyvsp[(1) - (1)].decl).type)) {
		Delete(SwigType_pop_function((yyvsp[(1) - (1)].decl).type));
	      } else if (SwigType_isarray((yyvsp[(1) - (1)].decl).type)) {
		SwigType *ta = SwigType_pop_arrays((yyvsp[(1) - (1)].decl).type);
		if (SwigType_isfunction((yyvsp[(1) - (1)].decl).type)) {
		  Delete(SwigType_pop_function((yyvsp[(1) - (1)].decl).type));
		} else {
		  (yyval.decl).parms = 0;
		}
		SwigType_push((yyvsp[(1) - (1)].decl).type,ta);
		Delete(ta);
	      } else {
		(yyval.decl).parms = 0;
	      }
            }
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 4889 "parser.y"
    {
   	      (yyval.decl).type = 0;
              (yyval.decl).id = 0;
	      (yyval.decl).parms = 0;
	      }
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 4897 "parser.y"
    {
              (yyval.decl) = (yyvsp[(2) - (2)].decl);
	      if ((yyval.decl).type) {
		SwigType_push((yyvsp[(1) - (2)].type),(yyval.decl).type);
		Delete((yyval.decl).type);
	      }
	      (yyval.decl).type = (yyvsp[(1) - (2)].type);
           }
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 4905 "parser.y"
    {
              (yyval.decl) = (yyvsp[(3) - (3)].decl);
	      SwigType_add_reference((yyvsp[(1) - (3)].type));
              if ((yyval.decl).type) {
		SwigType_push((yyvsp[(1) - (3)].type),(yyval.decl).type);
		Delete((yyval.decl).type);
	      }
	      (yyval.decl).type = (yyvsp[(1) - (3)].type);
           }
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 4914 "parser.y"
    {
              (yyval.decl) = (yyvsp[(3) - (3)].decl);
	      SwigType_add_rvalue_reference((yyvsp[(1) - (3)].type));
              if ((yyval.decl).type) {
		SwigType_push((yyvsp[(1) - (3)].type),(yyval.decl).type);
		Delete((yyval.decl).type);
	      }
	      (yyval.decl).type = (yyvsp[(1) - (3)].type);
           }
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 4923 "parser.y"
    {
              (yyval.decl) = (yyvsp[(1) - (1)].decl);
	      if (!(yyval.decl).type) (yyval.decl).type = NewStringEmpty();
           }
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 4927 "parser.y"
    {
	     (yyval.decl) = (yyvsp[(2) - (2)].decl);
	     (yyval.decl).type = NewStringEmpty();
	     SwigType_add_reference((yyval.decl).type);
	     if ((yyvsp[(2) - (2)].decl).type) {
	       SwigType_push((yyval.decl).type,(yyvsp[(2) - (2)].decl).type);
	       Delete((yyvsp[(2) - (2)].decl).type);
	     }
           }
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 4936 "parser.y"
    {
	     /* Introduced in C++11, move operator && */
             /* Adds one S/R conflict */
	     (yyval.decl) = (yyvsp[(2) - (2)].decl);
	     (yyval.decl).type = NewStringEmpty();
	     SwigType_add_rvalue_reference((yyval.decl).type);
	     if ((yyvsp[(2) - (2)].decl).type) {
	       SwigType_push((yyval.decl).type,(yyvsp[(2) - (2)].decl).type);
	       Delete((yyvsp[(2) - (2)].decl).type);
	     }
           }
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 4947 "parser.y"
    { 
	     SwigType *t = NewStringEmpty();

	     (yyval.decl) = (yyvsp[(3) - (3)].decl);
	     SwigType_add_memberpointer(t,(yyvsp[(1) - (3)].str));
	     if ((yyval.decl).type) {
	       SwigType_push(t,(yyval.decl).type);
	       Delete((yyval.decl).type);
	     }
	     (yyval.decl).type = t;
	     }
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 4958 "parser.y"
    { 
	     SwigType *t = NewStringEmpty();
	     (yyval.decl) = (yyvsp[(4) - (4)].decl);
	     SwigType_add_memberpointer(t,(yyvsp[(2) - (4)].str));
	     SwigType_push((yyvsp[(1) - (4)].type),t);
	     if ((yyval.decl).type) {
	       SwigType_push((yyvsp[(1) - (4)].type),(yyval.decl).type);
	       Delete((yyval.decl).type);
	     }
	     (yyval.decl).type = (yyvsp[(1) - (4)].type);
	     Delete(t);
	   }
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 4970 "parser.y"
    { 
	     (yyval.decl) = (yyvsp[(5) - (5)].decl);
	     SwigType_add_memberpointer((yyvsp[(1) - (5)].type),(yyvsp[(2) - (5)].str));
	     SwigType_add_reference((yyvsp[(1) - (5)].type));
	     if ((yyval.decl).type) {
	       SwigType_push((yyvsp[(1) - (5)].type),(yyval.decl).type);
	       Delete((yyval.decl).type);
	     }
	     (yyval.decl).type = (yyvsp[(1) - (5)].type);
	   }
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 4980 "parser.y"
    { 
	     SwigType *t = NewStringEmpty();
	     (yyval.decl) = (yyvsp[(4) - (4)].decl);
	     SwigType_add_memberpointer(t,(yyvsp[(1) - (4)].str));
	     SwigType_add_reference(t);
	     if ((yyval.decl).type) {
	       SwigType_push(t,(yyval.decl).type);
	       Delete((yyval.decl).type);
	     } 
	     (yyval.decl).type = t;
	   }
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 4994 "parser.y"
    {
              (yyval.decl) = (yyvsp[(5) - (5)].decl);
	      if ((yyval.decl).type) {
		SwigType_push((yyvsp[(1) - (5)].type),(yyval.decl).type);
		Delete((yyval.decl).type);
	      }
	      (yyval.decl).type = (yyvsp[(1) - (5)].type);
           }
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 5002 "parser.y"
    {
              (yyval.decl) = (yyvsp[(6) - (6)].decl);
	      SwigType_add_reference((yyvsp[(1) - (6)].type));
              if ((yyval.decl).type) {
		SwigType_push((yyvsp[(1) - (6)].type),(yyval.decl).type);
		Delete((yyval.decl).type);
	      }
	      (yyval.decl).type = (yyvsp[(1) - (6)].type);
           }
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 5011 "parser.y"
    {
              (yyval.decl) = (yyvsp[(6) - (6)].decl);
	      SwigType_add_rvalue_reference((yyvsp[(1) - (6)].type));
              if ((yyval.decl).type) {
		SwigType_push((yyvsp[(1) - (6)].type),(yyval.decl).type);
		Delete((yyval.decl).type);
	      }
	      (yyval.decl).type = (yyvsp[(1) - (6)].type);
           }
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 5020 "parser.y"
    {
              (yyval.decl) = (yyvsp[(4) - (4)].decl);
	      if (!(yyval.decl).type) (yyval.decl).type = NewStringEmpty();
           }
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 5024 "parser.y"
    {
	     (yyval.decl) = (yyvsp[(5) - (5)].decl);
	     (yyval.decl).type = NewStringEmpty();
	     SwigType_add_reference((yyval.decl).type);
	     if ((yyvsp[(5) - (5)].decl).type) {
	       SwigType_push((yyval.decl).type,(yyvsp[(5) - (5)].decl).type);
	       Delete((yyvsp[(5) - (5)].decl).type);
	     }
           }
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 5033 "parser.y"
    {
	     /* Introduced in C++11, move operator && */
             /* Adds one S/R conflict */
	     (yyval.decl) = (yyvsp[(5) - (5)].decl);
	     (yyval.decl).type = NewStringEmpty();
	     SwigType_add_rvalue_reference((yyval.decl).type);
	     if ((yyvsp[(5) - (5)].decl).type) {
	       SwigType_push((yyval.decl).type,(yyvsp[(5) - (5)].decl).type);
	       Delete((yyvsp[(5) - (5)].decl).type);
	     }
           }
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 5044 "parser.y"
    { 
	     SwigType *t = NewStringEmpty();

	     (yyval.decl) = (yyvsp[(6) - (6)].decl);
	     SwigType_add_memberpointer(t,(yyvsp[(1) - (6)].str));
	     if ((yyval.decl).type) {
	       SwigType_push(t,(yyval.decl).type);
	       Delete((yyval.decl).type);
	     }
	     (yyval.decl).type = t;
	     }
    break;

  case 315:

/* Line 1806 of yacc.c  */
#line 5055 "parser.y"
    { 
	     SwigType *t = NewStringEmpty();
	     (yyval.decl) = (yyvsp[(7) - (7)].decl);
	     SwigType_add_memberpointer(t,(yyvsp[(2) - (7)].str));
	     SwigType_push((yyvsp[(1) - (7)].type),t);
	     if ((yyval.decl).type) {
	       SwigType_push((yyvsp[(1) - (7)].type),(yyval.decl).type);
	       Delete((yyval.decl).type);
	     }
	     (yyval.decl).type = (yyvsp[(1) - (7)].type);
	     Delete(t);
	   }
    break;

  case 316:

/* Line 1806 of yacc.c  */
#line 5067 "parser.y"
    { 
	     (yyval.decl) = (yyvsp[(8) - (8)].decl);
	     SwigType_add_memberpointer((yyvsp[(1) - (8)].type),(yyvsp[(2) - (8)].str));
	     SwigType_add_reference((yyvsp[(1) - (8)].type));
	     if ((yyval.decl).type) {
	       SwigType_push((yyvsp[(1) - (8)].type),(yyval.decl).type);
	       Delete((yyval.decl).type);
	     }
	     (yyval.decl).type = (yyvsp[(1) - (8)].type);
	   }
    break;

  case 317:

/* Line 1806 of yacc.c  */
#line 5077 "parser.y"
    { 
	     (yyval.decl) = (yyvsp[(8) - (8)].decl);
	     SwigType_add_memberpointer((yyvsp[(1) - (8)].type),(yyvsp[(2) - (8)].str));
	     SwigType_add_rvalue_reference((yyvsp[(1) - (8)].type));
	     if ((yyval.decl).type) {
	       SwigType_push((yyvsp[(1) - (8)].type),(yyval.decl).type);
	       Delete((yyval.decl).type);
	     }
	     (yyval.decl).type = (yyvsp[(1) - (8)].type);
	   }
    break;

  case 318:

/* Line 1806 of yacc.c  */
#line 5087 "parser.y"
    { 
	     SwigType *t = NewStringEmpty();
	     (yyval.decl) = (yyvsp[(7) - (7)].decl);
	     SwigType_add_memberpointer(t,(yyvsp[(1) - (7)].str));
	     SwigType_add_reference(t);
	     if ((yyval.decl).type) {
	       SwigType_push(t,(yyval.decl).type);
	       Delete((yyval.decl).type);
	     } 
	     (yyval.decl).type = t;
	   }
    break;

  case 319:

/* Line 1806 of yacc.c  */
#line 5098 "parser.y"
    { 
	     SwigType *t = NewStringEmpty();
	     (yyval.decl) = (yyvsp[(7) - (7)].decl);
	     SwigType_add_memberpointer(t,(yyvsp[(1) - (7)].str));
	     SwigType_add_rvalue_reference(t);
	     if ((yyval.decl).type) {
	       SwigType_push(t,(yyval.decl).type);
	       Delete((yyval.decl).type);
	     } 
	     (yyval.decl).type = t;
	   }
    break;

  case 320:

/* Line 1806 of yacc.c  */
#line 5111 "parser.y"
    {
                /* Note: This is non-standard C.  Template declarator is allowed to follow an identifier */
                 (yyval.decl).id = Char((yyvsp[(1) - (1)].str));
		 (yyval.decl).type = 0;
		 (yyval.decl).parms = 0;
		 (yyval.decl).have_parms = 0;
                  }
    break;

  case 321:

/* Line 1806 of yacc.c  */
#line 5118 "parser.y"
    {
                  (yyval.decl).id = Char(NewStringf("~%s",(yyvsp[(2) - (2)].str)));
                  (yyval.decl).type = 0;
                  (yyval.decl).parms = 0;
                  (yyval.decl).have_parms = 0;
                  }
    break;

  case 322:

/* Line 1806 of yacc.c  */
#line 5126 "parser.y"
    {
                  (yyval.decl).id = Char((yyvsp[(2) - (3)].str));
                  (yyval.decl).type = 0;
                  (yyval.decl).parms = 0;
                  (yyval.decl).have_parms = 0;
                  }
    break;

  case 323:

/* Line 1806 of yacc.c  */
#line 5142 "parser.y"
    {
		    (yyval.decl) = (yyvsp[(3) - (4)].decl);
		    if ((yyval.decl).type) {
		      SwigType_push((yyvsp[(2) - (4)].type),(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = (yyvsp[(2) - (4)].type);
                  }
    break;

  case 324:

/* Line 1806 of yacc.c  */
#line 5150 "parser.y"
    {
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(4) - (5)].decl);
		    t = NewStringEmpty();
		    SwigType_add_memberpointer(t,(yyvsp[(2) - (5)].str));
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
		    }
    break;

  case 325:

/* Line 1806 of yacc.c  */
#line 5161 "parser.y"
    { 
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(1) - (3)].decl);
		    t = NewStringEmpty();
		    SwigType_add_array(t,"");
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
                  }
    break;

  case 326:

/* Line 1806 of yacc.c  */
#line 5172 "parser.y"
    { 
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(1) - (4)].decl);
		    t = NewStringEmpty();
		    SwigType_add_array(t,(yyvsp[(3) - (4)].dtype).val);
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
                  }
    break;

  case 327:

/* Line 1806 of yacc.c  */
#line 5183 "parser.y"
    {
		    SwigType *t;
                    (yyval.decl) = (yyvsp[(1) - (4)].decl);
		    t = NewStringEmpty();
		    SwigType_add_function(t,(yyvsp[(3) - (4)].pl));
		    if (!(yyval.decl).have_parms) {
		      (yyval.decl).parms = (yyvsp[(3) - (4)].pl);
		      (yyval.decl).have_parms = 1;
		    }
		    if (!(yyval.decl).type) {
		      (yyval.decl).type = t;
		    } else {
		      SwigType_push(t, (yyval.decl).type);
		      Delete((yyval.decl).type);
		      (yyval.decl).type = t;
		    }
		  }
    break;

  case 328:

/* Line 1806 of yacc.c  */
#line 5202 "parser.y"
    {
                /* Note: This is non-standard C.  Template declarator is allowed to follow an identifier */
                 (yyval.decl).id = Char((yyvsp[(1) - (1)].str));
		 (yyval.decl).type = 0;
		 (yyval.decl).parms = 0;
		 (yyval.decl).have_parms = 0;
                  }
    break;

  case 329:

/* Line 1806 of yacc.c  */
#line 5210 "parser.y"
    {
                  (yyval.decl).id = Char(NewStringf("~%s",(yyvsp[(2) - (2)].str)));
                  (yyval.decl).type = 0;
                  (yyval.decl).parms = 0;
                  (yyval.decl).have_parms = 0;
                  }
    break;

  case 330:

/* Line 1806 of yacc.c  */
#line 5227 "parser.y"
    {
		    (yyval.decl) = (yyvsp[(3) - (4)].decl);
		    if ((yyval.decl).type) {
		      SwigType_push((yyvsp[(2) - (4)].type),(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = (yyvsp[(2) - (4)].type);
                  }
    break;

  case 331:

/* Line 1806 of yacc.c  */
#line 5235 "parser.y"
    {
                    (yyval.decl) = (yyvsp[(3) - (4)].decl);
		    if (!(yyval.decl).type) {
		      (yyval.decl).type = NewStringEmpty();
		    }
		    SwigType_add_reference((yyval.decl).type);
                  }
    break;

  case 332:

/* Line 1806 of yacc.c  */
#line 5242 "parser.y"
    {
                    (yyval.decl) = (yyvsp[(3) - (4)].decl);
		    if (!(yyval.decl).type) {
		      (yyval.decl).type = NewStringEmpty();
		    }
		    SwigType_add_rvalue_reference((yyval.decl).type);
                  }
    break;

  case 333:

/* Line 1806 of yacc.c  */
#line 5249 "parser.y"
    {
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(4) - (5)].decl);
		    t = NewStringEmpty();
		    SwigType_add_memberpointer(t,(yyvsp[(2) - (5)].str));
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
		    }
    break;

  case 334:

/* Line 1806 of yacc.c  */
#line 5260 "parser.y"
    { 
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(1) - (3)].decl);
		    t = NewStringEmpty();
		    SwigType_add_array(t,"");
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
                  }
    break;

  case 335:

/* Line 1806 of yacc.c  */
#line 5271 "parser.y"
    { 
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(1) - (4)].decl);
		    t = NewStringEmpty();
		    SwigType_add_array(t,(yyvsp[(3) - (4)].dtype).val);
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
                  }
    break;

  case 336:

/* Line 1806 of yacc.c  */
#line 5282 "parser.y"
    {
		    SwigType *t;
                    (yyval.decl) = (yyvsp[(1) - (4)].decl);
		    t = NewStringEmpty();
		    SwigType_add_function(t,(yyvsp[(3) - (4)].pl));
		    if (!(yyval.decl).have_parms) {
		      (yyval.decl).parms = (yyvsp[(3) - (4)].pl);
		      (yyval.decl).have_parms = 1;
		    }
		    if (!(yyval.decl).type) {
		      (yyval.decl).type = t;
		    } else {
		      SwigType_push(t, (yyval.decl).type);
		      Delete((yyval.decl).type);
		      (yyval.decl).type = t;
		    }
                 }
    break;

  case 337:

/* Line 1806 of yacc.c  */
#line 5302 "parser.y"
    {
		    SwigType *t;
                    Append((yyvsp[(1) - (5)].str), " "); /* intervening space is mandatory */
                    Append((yyvsp[(1) - (5)].str), Char((yyvsp[(2) - (5)].id)));
		    (yyval.decl).id = Char((yyvsp[(1) - (5)].str));
		    t = NewStringEmpty();
		    SwigType_add_function(t,(yyvsp[(4) - (5)].pl));
		    if (!(yyval.decl).have_parms) {
		      (yyval.decl).parms = (yyvsp[(4) - (5)].pl);
		      (yyval.decl).have_parms = 1;
		    }
		    if (!(yyval.decl).type) {
		      (yyval.decl).type = t;
		    } else {
		      SwigType_push(t, (yyval.decl).type);
		      Delete((yyval.decl).type);
		      (yyval.decl).type = t;
		    }
		  }
    break;

  case 338:

/* Line 1806 of yacc.c  */
#line 5323 "parser.y"
    {
		    (yyval.decl).type = (yyvsp[(1) - (1)].type);
                    (yyval.decl).id = 0;
		    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
                  }
    break;

  case 339:

/* Line 1806 of yacc.c  */
#line 5329 "parser.y"
    { 
                     (yyval.decl) = (yyvsp[(2) - (2)].decl);
                     SwigType_push((yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].decl).type);
		     (yyval.decl).type = (yyvsp[(1) - (2)].type);
		     Delete((yyvsp[(2) - (2)].decl).type);
                  }
    break;

  case 340:

/* Line 1806 of yacc.c  */
#line 5335 "parser.y"
    {
		    (yyval.decl).type = (yyvsp[(1) - (2)].type);
		    SwigType_add_reference((yyval.decl).type);
		    (yyval.decl).id = 0;
		    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
		  }
    break;

  case 341:

/* Line 1806 of yacc.c  */
#line 5342 "parser.y"
    {
		    (yyval.decl).type = (yyvsp[(1) - (2)].type);
		    SwigType_add_rvalue_reference((yyval.decl).type);
		    (yyval.decl).id = 0;
		    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
		  }
    break;

  case 342:

/* Line 1806 of yacc.c  */
#line 5349 "parser.y"
    {
		    (yyval.decl) = (yyvsp[(3) - (3)].decl);
		    SwigType_add_reference((yyvsp[(1) - (3)].type));
		    if ((yyval.decl).type) {
		      SwigType_push((yyvsp[(1) - (3)].type),(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = (yyvsp[(1) - (3)].type);
                  }
    break;

  case 343:

/* Line 1806 of yacc.c  */
#line 5358 "parser.y"
    {
		    (yyval.decl) = (yyvsp[(3) - (3)].decl);
		    SwigType_add_rvalue_reference((yyvsp[(1) - (3)].type));
		    if ((yyval.decl).type) {
		      SwigType_push((yyvsp[(1) - (3)].type),(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = (yyvsp[(1) - (3)].type);
                  }
    break;

  case 344:

/* Line 1806 of yacc.c  */
#line 5367 "parser.y"
    {
		    (yyval.decl) = (yyvsp[(1) - (1)].decl);
                  }
    break;

  case 345:

/* Line 1806 of yacc.c  */
#line 5370 "parser.y"
    {
		    (yyval.decl) = (yyvsp[(2) - (2)].decl);
		    (yyval.decl).type = NewStringEmpty();
		    SwigType_add_reference((yyval.decl).type);
		    if ((yyvsp[(2) - (2)].decl).type) {
		      SwigType_push((yyval.decl).type,(yyvsp[(2) - (2)].decl).type);
		      Delete((yyvsp[(2) - (2)].decl).type);
		    }
                  }
    break;

  case 346:

/* Line 1806 of yacc.c  */
#line 5379 "parser.y"
    {
		    (yyval.decl) = (yyvsp[(2) - (2)].decl);
		    (yyval.decl).type = NewStringEmpty();
		    SwigType_add_rvalue_reference((yyval.decl).type);
		    if ((yyvsp[(2) - (2)].decl).type) {
		      SwigType_push((yyval.decl).type,(yyvsp[(2) - (2)].decl).type);
		      Delete((yyvsp[(2) - (2)].decl).type);
		    }
                  }
    break;

  case 347:

/* Line 1806 of yacc.c  */
#line 5388 "parser.y"
    {
                    (yyval.decl).id = 0;
                    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
                    (yyval.decl).type = NewStringEmpty();
		    SwigType_add_reference((yyval.decl).type);
                  }
    break;

  case 348:

/* Line 1806 of yacc.c  */
#line 5395 "parser.y"
    {
                    (yyval.decl).id = 0;
                    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
                    (yyval.decl).type = NewStringEmpty();
		    SwigType_add_rvalue_reference((yyval.decl).type);
                  }
    break;

  case 349:

/* Line 1806 of yacc.c  */
#line 5402 "parser.y"
    { 
		    (yyval.decl).type = NewStringEmpty();
                    SwigType_add_memberpointer((yyval.decl).type,(yyvsp[(1) - (2)].str));
                    (yyval.decl).id = 0;
                    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
      	          }
    break;

  case 350:

/* Line 1806 of yacc.c  */
#line 5409 "parser.y"
    { 
		    SwigType *t = NewStringEmpty();
                    (yyval.decl).type = (yyvsp[(1) - (3)].type);
		    (yyval.decl).id = 0;
		    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
		    SwigType_add_memberpointer(t,(yyvsp[(2) - (3)].str));
		    SwigType_push((yyval.decl).type,t);
		    Delete(t);
                  }
    break;

  case 351:

/* Line 1806 of yacc.c  */
#line 5419 "parser.y"
    { 
		    (yyval.decl) = (yyvsp[(4) - (4)].decl);
		    SwigType_add_memberpointer((yyvsp[(1) - (4)].type),(yyvsp[(2) - (4)].str));
		    if ((yyval.decl).type) {
		      SwigType_push((yyvsp[(1) - (4)].type),(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = (yyvsp[(1) - (4)].type);
                  }
    break;

  case 352:

/* Line 1806 of yacc.c  */
#line 5430 "parser.y"
    { 
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(1) - (3)].decl);
		    t = NewStringEmpty();
		    SwigType_add_array(t,"");
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
                  }
    break;

  case 353:

/* Line 1806 of yacc.c  */
#line 5441 "parser.y"
    { 
		    SwigType *t;
		    (yyval.decl) = (yyvsp[(1) - (4)].decl);
		    t = NewStringEmpty();
		    SwigType_add_array(t,(yyvsp[(3) - (4)].dtype).val);
		    if ((yyval.decl).type) {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		    }
		    (yyval.decl).type = t;
                  }
    break;

  case 354:

/* Line 1806 of yacc.c  */
#line 5452 "parser.y"
    { 
		    (yyval.decl).type = NewStringEmpty();
		    (yyval.decl).id = 0;
		    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
		    SwigType_add_array((yyval.decl).type,"");
                  }
    break;

  case 355:

/* Line 1806 of yacc.c  */
#line 5459 "parser.y"
    { 
		    (yyval.decl).type = NewStringEmpty();
		    (yyval.decl).id = 0;
		    (yyval.decl).parms = 0;
		    (yyval.decl).have_parms = 0;
		    SwigType_add_array((yyval.decl).type,(yyvsp[(2) - (3)].dtype).val);
		  }
    break;

  case 356:

/* Line 1806 of yacc.c  */
#line 5466 "parser.y"
    {
                    (yyval.decl) = (yyvsp[(2) - (3)].decl);
		  }
    break;

  case 357:

/* Line 1806 of yacc.c  */
#line 5469 "parser.y"
    {
		    SwigType *t;
                    (yyval.decl) = (yyvsp[(1) - (4)].decl);
		    t = NewStringEmpty();
                    SwigType_add_function(t,(yyvsp[(3) - (4)].pl));
		    if (!(yyval.decl).type) {
		      (yyval.decl).type = t;
		    } else {
		      SwigType_push(t,(yyval.decl).type);
		      Delete((yyval.decl).type);
		      (yyval.decl).type = t;
		    }
		    if (!(yyval.decl).have_parms) {
		      (yyval.decl).parms = (yyvsp[(3) - (4)].pl);
		      (yyval.decl).have_parms = 1;
		    }
		  }
    break;

  case 358:

/* Line 1806 of yacc.c  */
#line 5486 "parser.y"
    {
                    (yyval.decl).type = NewStringEmpty();
                    SwigType_add_function((yyval.decl).type,(yyvsp[(2) - (3)].pl));
		    (yyval.decl).parms = (yyvsp[(2) - (3)].pl);
		    (yyval.decl).have_parms = 1;
		    (yyval.decl).id = 0;
                  }
    break;

  case 359:

/* Line 1806 of yacc.c  */
#line 5496 "parser.y"
    { 
             (yyval.type) = NewStringEmpty();
             SwigType_add_pointer((yyval.type));
	     SwigType_push((yyval.type),(yyvsp[(2) - (3)].str));
	     SwigType_push((yyval.type),(yyvsp[(3) - (3)].type));
	     Delete((yyvsp[(3) - (3)].type));
           }
    break;

  case 360:

/* Line 1806 of yacc.c  */
#line 5503 "parser.y"
    {
	     (yyval.type) = NewStringEmpty();
	     SwigType_add_pointer((yyval.type));
	     SwigType_push((yyval.type),(yyvsp[(2) - (2)].type));
	     Delete((yyvsp[(2) - (2)].type));
	   }
    break;

  case 361:

/* Line 1806 of yacc.c  */
#line 5509 "parser.y"
    { 
	     (yyval.type) = NewStringEmpty();
	     SwigType_add_pointer((yyval.type));
	     SwigType_push((yyval.type),(yyvsp[(2) - (2)].str));
           }
    break;

  case 362:

/* Line 1806 of yacc.c  */
#line 5514 "parser.y"
    {
	     (yyval.type) = NewStringEmpty();
	     SwigType_add_pointer((yyval.type));
           }
    break;

  case 363:

/* Line 1806 of yacc.c  */
#line 5520 "parser.y"
    {
	          (yyval.str) = NewStringEmpty();
	          if ((yyvsp[(1) - (1)].id)) SwigType_add_qualifier((yyval.str),(yyvsp[(1) - (1)].id));
               }
    break;

  case 364:

/* Line 1806 of yacc.c  */
#line 5524 "parser.y"
    {
		  (yyval.str) = (yyvsp[(2) - (2)].str);
	          if ((yyvsp[(1) - (2)].id)) SwigType_add_qualifier((yyval.str),(yyvsp[(1) - (2)].id));
               }
    break;

  case 365:

/* Line 1806 of yacc.c  */
#line 5530 "parser.y"
    { (yyval.id) = "const"; }
    break;

  case 366:

/* Line 1806 of yacc.c  */
#line 5531 "parser.y"
    { (yyval.id) = "volatile"; }
    break;

  case 367:

/* Line 1806 of yacc.c  */
#line 5532 "parser.y"
    { (yyval.id) = 0; }
    break;

  case 368:

/* Line 1806 of yacc.c  */
#line 5538 "parser.y"
    {
                   (yyval.type) = (yyvsp[(1) - (1)].type);
                   Replace((yyval.type),"typename ","", DOH_REPLACE_ANY);
                }
    break;

  case 369:

/* Line 1806 of yacc.c  */
#line 5544 "parser.y"
    {
                   (yyval.type) = (yyvsp[(2) - (2)].type);
	           SwigType_push((yyval.type),(yyvsp[(1) - (2)].str));
               }
    break;

  case 370:

/* Line 1806 of yacc.c  */
#line 5548 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type); }
    break;

  case 371:

/* Line 1806 of yacc.c  */
#line 5549 "parser.y"
    {
		  (yyval.type) = (yyvsp[(1) - (2)].type);
	          SwigType_push((yyval.type),(yyvsp[(2) - (2)].str));
	       }
    break;

  case 372:

/* Line 1806 of yacc.c  */
#line 5553 "parser.y"
    {
		  (yyval.type) = (yyvsp[(2) - (3)].type);
	          SwigType_push((yyval.type),(yyvsp[(3) - (3)].str));
	          SwigType_push((yyval.type),(yyvsp[(1) - (3)].str));
	       }
    break;

  case 373:

/* Line 1806 of yacc.c  */
#line 5560 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type);
                  /* Printf(stdout,"primitive = '%s'\n", $$);*/
               }
    break;

  case 374:

/* Line 1806 of yacc.c  */
#line 5563 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type); }
    break;

  case 375:

/* Line 1806 of yacc.c  */
#line 5564 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type); }
    break;

  case 376:

/* Line 1806 of yacc.c  */
#line 5565 "parser.y"
    { (yyval.type) = NewStringf("%s%s",(yyvsp[(1) - (2)].type),(yyvsp[(2) - (2)].id)); }
    break;

  case 377:

/* Line 1806 of yacc.c  */
#line 5566 "parser.y"
    { (yyval.type) = NewStringf("enum %s", (yyvsp[(2) - (2)].str)); }
    break;

  case 378:

/* Line 1806 of yacc.c  */
#line 5567 "parser.y"
    { (yyval.type) = (yyvsp[(1) - (1)].type); }
    break;

  case 379:

/* Line 1806 of yacc.c  */
#line 5569 "parser.y"
    {
		  (yyval.type) = (yyvsp[(1) - (1)].str);
               }
    break;

  case 380:

/* Line 1806 of yacc.c  */
#line 5572 "parser.y"
    { 
		 (yyval.type) = NewStringf("%s %s", (yyvsp[(1) - (2)].id), (yyvsp[(2) - (2)].str));
               }
    break;

  case 381:

/* Line 1806 of yacc.c  */
#line 5575 "parser.y"
    {
                 (yyval.type) = (yyvsp[(1) - (1)].type);
               }
    break;

  case 382:

/* Line 1806 of yacc.c  */
#line 5580 "parser.y"
    {
                 Node *n = Swig_symbol_clookup((yyvsp[(3) - (4)].str),0);
                 if (!n) {
		   Swig_error(cparse_file, cparse_line, "Identifier %s not defined.\n", (yyvsp[(3) - (4)].str));
                   (yyval.type) = (yyvsp[(3) - (4)].str);
                 } else {
                   (yyval.type) = Getattr(n, "type");
                 }
               }
    break;

  case 383:

/* Line 1806 of yacc.c  */
#line 5591 "parser.y"
    {
		 if (!(yyvsp[(1) - (1)].ptype).type) (yyvsp[(1) - (1)].ptype).type = NewString("int");
		 if ((yyvsp[(1) - (1)].ptype).us) {
		   (yyval.type) = NewStringf("%s %s", (yyvsp[(1) - (1)].ptype).us, (yyvsp[(1) - (1)].ptype).type);
		   Delete((yyvsp[(1) - (1)].ptype).us);
                   Delete((yyvsp[(1) - (1)].ptype).type);
		 } else {
                   (yyval.type) = (yyvsp[(1) - (1)].ptype).type;
		 }
		 if (Cmp((yyval.type),"signed int") == 0) {
		   Delete((yyval.type));
		   (yyval.type) = NewString("int");
                 } else if (Cmp((yyval.type),"signed long") == 0) {
		   Delete((yyval.type));
                   (yyval.type) = NewString("long");
                 } else if (Cmp((yyval.type),"signed short") == 0) {
		   Delete((yyval.type));
		   (yyval.type) = NewString("short");
		 } else if (Cmp((yyval.type),"signed long long") == 0) {
		   Delete((yyval.type));
		   (yyval.type) = NewString("long long");
		 }
               }
    break;

  case 384:

/* Line 1806 of yacc.c  */
#line 5616 "parser.y"
    { 
                 (yyval.ptype) = (yyvsp[(1) - (1)].ptype);
               }
    break;

  case 385:

/* Line 1806 of yacc.c  */
#line 5619 "parser.y"
    {
                    if ((yyvsp[(1) - (2)].ptype).us && (yyvsp[(2) - (2)].ptype).us) {
		      Swig_error(cparse_file, cparse_line, "Extra %s specifier.\n", (yyvsp[(2) - (2)].ptype).us);
		    }
                    (yyval.ptype) = (yyvsp[(2) - (2)].ptype);
                    if ((yyvsp[(1) - (2)].ptype).us) (yyval.ptype).us = (yyvsp[(1) - (2)].ptype).us;
		    if ((yyvsp[(1) - (2)].ptype).type) {
		      if (!(yyvsp[(2) - (2)].ptype).type) (yyval.ptype).type = (yyvsp[(1) - (2)].ptype).type;
		      else {
			int err = 0;
			if ((Cmp((yyvsp[(1) - (2)].ptype).type,"long") == 0)) {
			  if ((Cmp((yyvsp[(2) - (2)].ptype).type,"long") == 0) || (Strncmp((yyvsp[(2) - (2)].ptype).type,"double",6) == 0)) {
			    (yyval.ptype).type = NewStringf("long %s", (yyvsp[(2) - (2)].ptype).type);
			  } else if (Cmp((yyvsp[(2) - (2)].ptype).type,"int") == 0) {
			    (yyval.ptype).type = (yyvsp[(1) - (2)].ptype).type;
			  } else {
			    err = 1;
			  }
			} else if ((Cmp((yyvsp[(1) - (2)].ptype).type,"short")) == 0) {
			  if (Cmp((yyvsp[(2) - (2)].ptype).type,"int") == 0) {
			    (yyval.ptype).type = (yyvsp[(1) - (2)].ptype).type;
			  } else {
			    err = 1;
			  }
			} else if (Cmp((yyvsp[(1) - (2)].ptype).type,"int") == 0) {
			  (yyval.ptype).type = (yyvsp[(2) - (2)].ptype).type;
			} else if (Cmp((yyvsp[(1) - (2)].ptype).type,"double") == 0) {
			  if (Cmp((yyvsp[(2) - (2)].ptype).type,"long") == 0) {
			    (yyval.ptype).type = NewString("long double");
			  } else if (Cmp((yyvsp[(2) - (2)].ptype).type,"complex") == 0) {
			    (yyval.ptype).type = NewString("double complex");
			  } else {
			    err = 1;
			  }
			} else if (Cmp((yyvsp[(1) - (2)].ptype).type,"float") == 0) {
			  if (Cmp((yyvsp[(2) - (2)].ptype).type,"complex") == 0) {
			    (yyval.ptype).type = NewString("float complex");
			  } else {
			    err = 1;
			  }
			} else if (Cmp((yyvsp[(1) - (2)].ptype).type,"complex") == 0) {
			  (yyval.ptype).type = NewStringf("%s complex", (yyvsp[(2) - (2)].ptype).type);
			} else {
			  err = 1;
			}
			if (err) {
			  Swig_error(cparse_file, cparse_line, "Extra %s specifier.\n", (yyvsp[(1) - (2)].ptype).type);
			}
		      }
		    }
               }
    break;

  case 386:

/* Line 1806 of yacc.c  */
#line 5673 "parser.y"
    { 
		    (yyval.ptype).type = NewString("int");
                    (yyval.ptype).us = 0;
               }
    break;

  case 387:

/* Line 1806 of yacc.c  */
#line 5677 "parser.y"
    { 
                    (yyval.ptype).type = NewString("short");
                    (yyval.ptype).us = 0;
                }
    break;

  case 388:

/* Line 1806 of yacc.c  */
#line 5681 "parser.y"
    { 
                    (yyval.ptype).type = NewString("long");
                    (yyval.ptype).us = 0;
                }
    break;

  case 389:

/* Line 1806 of yacc.c  */
#line 5685 "parser.y"
    { 
                    (yyval.ptype).type = NewString("char");
                    (yyval.ptype).us = 0;
                }
    break;

  case 390:

/* Line 1806 of yacc.c  */
#line 5689 "parser.y"
    { 
                    (yyval.ptype).type = NewString("wchar_t");
                    (yyval.ptype).us = 0;
                }
    break;

  case 391:

/* Line 1806 of yacc.c  */
#line 5693 "parser.y"
    { 
                    (yyval.ptype).type = NewString("float");
                    (yyval.ptype).us = 0;
                }
    break;

  case 392:

/* Line 1806 of yacc.c  */
#line 5697 "parser.y"
    { 
                    (yyval.ptype).type = NewString("double");
                    (yyval.ptype).us = 0;
                }
    break;

  case 393:

/* Line 1806 of yacc.c  */
#line 5701 "parser.y"
    { 
                    (yyval.ptype).us = NewString("signed");
                    (yyval.ptype).type = 0;
                }
    break;

  case 394:

/* Line 1806 of yacc.c  */
#line 5705 "parser.y"
    { 
                    (yyval.ptype).us = NewString("unsigned");
                    (yyval.ptype).type = 0;
                }
    break;

  case 395:

/* Line 1806 of yacc.c  */
#line 5709 "parser.y"
    { 
                    (yyval.ptype).type = NewString("complex");
                    (yyval.ptype).us = 0;
                }
    break;

  case 396:

/* Line 1806 of yacc.c  */
#line 5713 "parser.y"
    { 
                    (yyval.ptype).type = NewString("__int8");
                    (yyval.ptype).us = 0;
                }
    break;

  case 397:

/* Line 1806 of yacc.c  */
#line 5717 "parser.y"
    { 
                    (yyval.ptype).type = NewString("__int16");
                    (yyval.ptype).us = 0;
                }
    break;

  case 398:

/* Line 1806 of yacc.c  */
#line 5721 "parser.y"
    { 
                    (yyval.ptype).type = NewString("__int32");
                    (yyval.ptype).us = 0;
                }
    break;

  case 399:

/* Line 1806 of yacc.c  */
#line 5725 "parser.y"
    { 
                    (yyval.ptype).type = NewString("__int64");
                    (yyval.ptype).us = 0;
                }
    break;

  case 400:

/* Line 1806 of yacc.c  */
#line 5731 "parser.y"
    { /* scanner_check_typedef(); */ }
    break;

  case 401:

/* Line 1806 of yacc.c  */
#line 5731 "parser.y"
    {
                   (yyval.dtype) = (yyvsp[(2) - (2)].dtype);
		   if ((yyval.dtype).type == T_STRING) {
		     (yyval.dtype).rawval = NewStringf("\"%(escape)s\"",(yyval.dtype).val);
		   } else if ((yyval.dtype).type != T_CHAR && (yyval.dtype).type != T_WSTRING && (yyval.dtype).type != T_WCHAR) {
		     (yyval.dtype).rawval = 0;
		   }
		   (yyval.dtype).qualifier = 0;
		   (yyval.dtype).bitfield = 0;
		   (yyval.dtype).throws = 0;
		   (yyval.dtype).throwf = 0;
		   (yyval.dtype).nexcept = 0;
		   scanner_ignore_typedef();
                }
    break;

  case 402:

/* Line 1806 of yacc.c  */
#line 5745 "parser.y"
    {
		  (yyval.dtype) = (yyvsp[(1) - (1)].dtype);
		}
    break;

  case 403:

/* Line 1806 of yacc.c  */
#line 5761 "parser.y"
    {
		  (yyval.dtype) = (yyvsp[(1) - (1)].dtype);
		}
    break;

  case 404:

/* Line 1806 of yacc.c  */
#line 5764 "parser.y"
    {
		  (yyval.dtype) = (yyvsp[(1) - (1)].dtype);
		}
    break;

  case 405:

/* Line 1806 of yacc.c  */
#line 5770 "parser.y"
    {
		  (yyval.dtype).val = NewString("delete");
		  (yyval.dtype).rawval = 0;
		  (yyval.dtype).type = T_STRING;
		  (yyval.dtype).qualifier = 0;
		  (yyval.dtype).bitfield = 0;
		  (yyval.dtype).throws = 0;
		  (yyval.dtype).throwf = 0;
		  (yyval.dtype).nexcept = 0;
		}
    break;

  case 406:

/* Line 1806 of yacc.c  */
#line 5783 "parser.y"
    {
		  (yyval.dtype).val = NewString("default");
		  (yyval.dtype).rawval = 0;
		  (yyval.dtype).type = T_STRING;
		  (yyval.dtype).qualifier = 0;
		  (yyval.dtype).bitfield = 0;
		  (yyval.dtype).throws = 0;
		  (yyval.dtype).throwf = 0;
		  (yyval.dtype).nexcept = 0;
		}
    break;

  case 407:

/* Line 1806 of yacc.c  */
#line 5797 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].id); }
    break;

  case 408:

/* Line 1806 of yacc.c  */
#line 5798 "parser.y"
    { (yyval.id) = (char *) 0;}
    break;

  case 409:

/* Line 1806 of yacc.c  */
#line 5801 "parser.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); }
    break;

  case 410:

/* Line 1806 of yacc.c  */
#line 5802 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 411:

/* Line 1806 of yacc.c  */
#line 5806 "parser.y"
    {
		 Node *leftSibling = Getattr((yyvsp[(1) - (5)].node),"_last");
		 set_nextSibling(leftSibling,(yyvsp[(4) - (5)].node));
		 Setattr((yyvsp[(1) - (5)].node),"_last",(yyvsp[(4) - (5)].node));
		 (yyval.node) = (yyvsp[(1) - (5)].node);
	       }
    break;

  case 412:

/* Line 1806 of yacc.c  */
#line 5812 "parser.y"
    {
		 (yyval.node) = (yyvsp[(1) - (3)].node);
	       }
    break;

  case 413:

/* Line 1806 of yacc.c  */
#line 5815 "parser.y"
    {
		 Setattr((yyvsp[(2) - (3)].node),"_last",(yyvsp[(2) - (3)].node));
		 (yyval.node) = (yyvsp[(2) - (3)].node);
	       }
    break;

  case 414:

/* Line 1806 of yacc.c  */
#line 5819 "parser.y"
    {
		 (yyval.node) = 0;
	       }
    break;

  case 415:

/* Line 1806 of yacc.c  */
#line 5824 "parser.y"
    {
		   SwigType *type = NewSwigType(T_INT);
		   (yyval.node) = new_node("enumitem");
		   Setattr((yyval.node),"name",(yyvsp[(1) - (1)].id));
		   Setattr((yyval.node),"type",type);
		   SetFlag((yyval.node),"feature:immutable");
		   Delete(type);
		 }
    break;

  case 416:

/* Line 1806 of yacc.c  */
#line 5832 "parser.y"
    {
		   SwigType *type = NewSwigType((yyvsp[(3) - (3)].dtype).type == T_BOOL ? T_BOOL : ((yyvsp[(3) - (3)].dtype).type == T_CHAR ? T_CHAR : T_INT));
		   (yyval.node) = new_node("enumitem");
		   Setattr((yyval.node),"name",(yyvsp[(1) - (3)].id));
		   Setattr((yyval.node),"type",type);
		   SetFlag((yyval.node),"feature:immutable");
		   Setattr((yyval.node),"enumvalue", (yyvsp[(3) - (3)].dtype).val);
		   Setattr((yyval.node),"value",(yyvsp[(1) - (3)].id));
		   Delete(type);
                 }
    break;

  case 417:

/* Line 1806 of yacc.c  */
#line 5844 "parser.y"
    {
                   (yyval.dtype) = (yyvsp[(1) - (1)].dtype);
		   if (((yyval.dtype).type != T_INT) && ((yyval.dtype).type != T_UINT) &&
		       ((yyval.dtype).type != T_LONG) && ((yyval.dtype).type != T_ULONG) &&
		       ((yyval.dtype).type != T_LONGLONG) && ((yyval.dtype).type != T_ULONGLONG) &&
		       ((yyval.dtype).type != T_SHORT) && ((yyval.dtype).type != T_USHORT) &&
		       ((yyval.dtype).type != T_SCHAR) && ((yyval.dtype).type != T_UCHAR) &&
		       ((yyval.dtype).type != T_CHAR) && ((yyval.dtype).type != T_BOOL)) {
		     Swig_error(cparse_file,cparse_line,"Type error. Expecting an integral type\n");
		   }
                }
    break;

  case 418:

/* Line 1806 of yacc.c  */
#line 5859 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 419:

/* Line 1806 of yacc.c  */
#line 5860 "parser.y"
    {
		 Node *n;
		 (yyval.dtype).val = (yyvsp[(1) - (1)].type);
		 (yyval.dtype).type = T_INT;
		 /* Check if value is in scope */
		 n = Swig_symbol_clookup((yyvsp[(1) - (1)].type),0);
		 if (n) {
                   /* A band-aid for enum values used in expressions. */
                   if (Strcmp(nodeType(n),"enumitem") == 0) {
                     String *q = Swig_symbol_qualified(n);
                     if (q) {
                       (yyval.dtype).val = NewStringf("%s::%s", q, Getattr(n,"name"));
                       Delete(q);
                     }
                   }
		 }
               }
    break;

  case 420:

/* Line 1806 of yacc.c  */
#line 5879 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 421:

/* Line 1806 of yacc.c  */
#line 5880 "parser.y"
    {
		    (yyval.dtype).val = NewString((yyvsp[(1) - (1)].id));
                    (yyval.dtype).type = T_STRING;
               }
    break;

  case 422:

/* Line 1806 of yacc.c  */
#line 5884 "parser.y"
    {
		  SwigType_push((yyvsp[(3) - (5)].type),(yyvsp[(4) - (5)].decl).type);
		  (yyval.dtype).val = NewStringf("sizeof(%s)",SwigType_str((yyvsp[(3) - (5)].type),0));
		  (yyval.dtype).type = T_ULONG;
               }
    break;

  case 423:

/* Line 1806 of yacc.c  */
#line 5889 "parser.y"
    {
		  SwigType_push((yyvsp[(6) - (8)].type),(yyvsp[(7) - (8)].decl).type);
		  (yyval.dtype).val = NewStringf("sizeof...(%s)",SwigType_str((yyvsp[(6) - (8)].type),0));
		  (yyval.dtype).type = T_ULONG;
               }
    break;

  case 424:

/* Line 1806 of yacc.c  */
#line 5894 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 425:

/* Line 1806 of yacc.c  */
#line 5895 "parser.y"
    {
		    (yyval.dtype).val = NewString((yyvsp[(1) - (1)].id));
		    (yyval.dtype).rawval = NewStringf("L\"%s\"", (yyval.dtype).val);
                    (yyval.dtype).type = T_WSTRING;
	       }
    break;

  case 426:

/* Line 1806 of yacc.c  */
#line 5900 "parser.y"
    {
		  (yyval.dtype).val = NewString((yyvsp[(1) - (1)].str));
		  if (Len((yyval.dtype).val)) {
		    (yyval.dtype).rawval = NewStringf("'%(escape)s'", (yyval.dtype).val);
		  } else {
		    (yyval.dtype).rawval = NewString("'\\0'");
		  }
		  (yyval.dtype).type = T_CHAR;
		  (yyval.dtype).bitfield = 0;
		  (yyval.dtype).throws = 0;
		  (yyval.dtype).throwf = 0;
		  (yyval.dtype).nexcept = 0;
	       }
    break;

  case 427:

/* Line 1806 of yacc.c  */
#line 5913 "parser.y"
    {
		  (yyval.dtype).val = NewString((yyvsp[(1) - (1)].str));
		  if (Len((yyval.dtype).val)) {
		    (yyval.dtype).rawval = NewStringf("L\'%s\'", (yyval.dtype).val);
		  } else {
		    (yyval.dtype).rawval = NewString("L'\\0'");
		  }
		  (yyval.dtype).type = T_WCHAR;
		  (yyval.dtype).bitfield = 0;
		  (yyval.dtype).throws = 0;
		  (yyval.dtype).throwf = 0;
		  (yyval.dtype).nexcept = 0;
	       }
    break;

  case 428:

/* Line 1806 of yacc.c  */
#line 5928 "parser.y"
    {
   	            (yyval.dtype).val = NewStringf("(%s)",(yyvsp[(2) - (3)].dtype).val);
		    (yyval.dtype).type = (yyvsp[(2) - (3)].dtype).type;
   	       }
    break;

  case 429:

/* Line 1806 of yacc.c  */
#line 5935 "parser.y"
    {
                 (yyval.dtype) = (yyvsp[(4) - (4)].dtype);
		 if ((yyvsp[(4) - (4)].dtype).type != T_STRING) {
		   switch ((yyvsp[(2) - (4)].dtype).type) {
		     case T_FLOAT:
		     case T_DOUBLE:
		     case T_LONGDOUBLE:
		     case T_FLTCPLX:
		     case T_DBLCPLX:
		       (yyval.dtype).val = NewStringf("(%s)%s", (yyvsp[(2) - (4)].dtype).val, (yyvsp[(4) - (4)].dtype).val); /* SwigType_str and decimal points don't mix! */
		       break;
		     default:
		       (yyval.dtype).val = NewStringf("(%s) %s", SwigType_str((yyvsp[(2) - (4)].dtype).val,0), (yyvsp[(4) - (4)].dtype).val);
		       break;
		   }
		 }
 	       }
    break;

  case 430:

/* Line 1806 of yacc.c  */
#line 5952 "parser.y"
    {
                 (yyval.dtype) = (yyvsp[(5) - (5)].dtype);
		 if ((yyvsp[(5) - (5)].dtype).type != T_STRING) {
		   SwigType_push((yyvsp[(2) - (5)].dtype).val,(yyvsp[(3) - (5)].type));
		   (yyval.dtype).val = NewStringf("(%s) %s", SwigType_str((yyvsp[(2) - (5)].dtype).val,0), (yyvsp[(5) - (5)].dtype).val);
		 }
 	       }
    break;

  case 431:

/* Line 1806 of yacc.c  */
#line 5959 "parser.y"
    {
                 (yyval.dtype) = (yyvsp[(5) - (5)].dtype);
		 if ((yyvsp[(5) - (5)].dtype).type != T_STRING) {
		   SwigType_add_reference((yyvsp[(2) - (5)].dtype).val);
		   (yyval.dtype).val = NewStringf("(%s) %s", SwigType_str((yyvsp[(2) - (5)].dtype).val,0), (yyvsp[(5) - (5)].dtype).val);
		 }
 	       }
    break;

  case 432:

/* Line 1806 of yacc.c  */
#line 5966 "parser.y"
    {
                 (yyval.dtype) = (yyvsp[(5) - (5)].dtype);
		 if ((yyvsp[(5) - (5)].dtype).type != T_STRING) {
		   SwigType_add_rvalue_reference((yyvsp[(2) - (5)].dtype).val);
		   (yyval.dtype).val = NewStringf("(%s) %s", SwigType_str((yyvsp[(2) - (5)].dtype).val,0), (yyvsp[(5) - (5)].dtype).val);
		 }
 	       }
    break;

  case 433:

/* Line 1806 of yacc.c  */
#line 5973 "parser.y"
    {
                 (yyval.dtype) = (yyvsp[(6) - (6)].dtype);
		 if ((yyvsp[(6) - (6)].dtype).type != T_STRING) {
		   SwigType_push((yyvsp[(2) - (6)].dtype).val,(yyvsp[(3) - (6)].type));
		   SwigType_add_reference((yyvsp[(2) - (6)].dtype).val);
		   (yyval.dtype).val = NewStringf("(%s) %s", SwigType_str((yyvsp[(2) - (6)].dtype).val,0), (yyvsp[(6) - (6)].dtype).val);
		 }
 	       }
    break;

  case 434:

/* Line 1806 of yacc.c  */
#line 5981 "parser.y"
    {
                 (yyval.dtype) = (yyvsp[(6) - (6)].dtype);
		 if ((yyvsp[(6) - (6)].dtype).type != T_STRING) {
		   SwigType_push((yyvsp[(2) - (6)].dtype).val,(yyvsp[(3) - (6)].type));
		   SwigType_add_rvalue_reference((yyvsp[(2) - (6)].dtype).val);
		   (yyval.dtype).val = NewStringf("(%s) %s", SwigType_str((yyvsp[(2) - (6)].dtype).val,0), (yyvsp[(6) - (6)].dtype).val);
		 }
 	       }
    break;

  case 435:

/* Line 1806 of yacc.c  */
#line 5989 "parser.y"
    {
		 (yyval.dtype) = (yyvsp[(2) - (2)].dtype);
                 (yyval.dtype).val = NewStringf("&%s",(yyvsp[(2) - (2)].dtype).val);
	       }
    break;

  case 436:

/* Line 1806 of yacc.c  */
#line 5993 "parser.y"
    {
		 (yyval.dtype) = (yyvsp[(2) - (2)].dtype);
                 (yyval.dtype).val = NewStringf("&&%s",(yyvsp[(2) - (2)].dtype).val);
	       }
    break;

  case 437:

/* Line 1806 of yacc.c  */
#line 5997 "parser.y"
    {
		 (yyval.dtype) = (yyvsp[(2) - (2)].dtype);
                 (yyval.dtype).val = NewStringf("*%s",(yyvsp[(2) - (2)].dtype).val);
	       }
    break;

  case 438:

/* Line 1806 of yacc.c  */
#line 6003 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 439:

/* Line 1806 of yacc.c  */
#line 6004 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 440:

/* Line 1806 of yacc.c  */
#line 6005 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 441:

/* Line 1806 of yacc.c  */
#line 6006 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 442:

/* Line 1806 of yacc.c  */
#line 6007 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 443:

/* Line 1806 of yacc.c  */
#line 6008 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 444:

/* Line 1806 of yacc.c  */
#line 6009 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 445:

/* Line 1806 of yacc.c  */
#line 6010 "parser.y"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype); }
    break;

  case 446:

/* Line 1806 of yacc.c  */
#line 6013 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s+%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 447:

/* Line 1806 of yacc.c  */
#line 6017 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s-%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 448:

/* Line 1806 of yacc.c  */
#line 6021 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s*%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 449:

/* Line 1806 of yacc.c  */
#line 6025 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s/%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 450:

/* Line 1806 of yacc.c  */
#line 6029 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s%%%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 451:

/* Line 1806 of yacc.c  */
#line 6033 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s&%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 452:

/* Line 1806 of yacc.c  */
#line 6037 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s|%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 453:

/* Line 1806 of yacc.c  */
#line 6041 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s^%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote((yyvsp[(1) - (3)].dtype).type,(yyvsp[(3) - (3)].dtype).type);
	       }
    break;

  case 454:

/* Line 1806 of yacc.c  */
#line 6045 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s << %s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote_type((yyvsp[(1) - (3)].dtype).type);
	       }
    break;

  case 455:

/* Line 1806 of yacc.c  */
#line 6049 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s >> %s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = promote_type((yyvsp[(1) - (3)].dtype).type);
	       }
    break;

  case 456:

/* Line 1806 of yacc.c  */
#line 6053 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s&&%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = cparse_cplusplus ? T_BOOL : T_INT;
	       }
    break;

  case 457:

/* Line 1806 of yacc.c  */
#line 6057 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s||%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = cparse_cplusplus ? T_BOOL : T_INT;
	       }
    break;

  case 458:

/* Line 1806 of yacc.c  */
#line 6061 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s==%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = cparse_cplusplus ? T_BOOL : T_INT;
	       }
    break;

  case 459:

/* Line 1806 of yacc.c  */
#line 6065 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s!=%s",(yyvsp[(1) - (3)].dtype).val,(yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = cparse_cplusplus ? T_BOOL : T_INT;
	       }
    break;

  case 460:

/* Line 1806 of yacc.c  */
#line 6079 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s >= %s", (yyvsp[(1) - (3)].dtype).val, (yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = cparse_cplusplus ? T_BOOL : T_INT;
	       }
    break;

  case 461:

/* Line 1806 of yacc.c  */
#line 6083 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s <= %s", (yyvsp[(1) - (3)].dtype).val, (yyvsp[(3) - (3)].dtype).val);
		 (yyval.dtype).type = cparse_cplusplus ? T_BOOL : T_INT;
	       }
    break;

  case 462:

/* Line 1806 of yacc.c  */
#line 6087 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("%s?%s:%s", (yyvsp[(1) - (5)].dtype).val, (yyvsp[(3) - (5)].dtype).val, (yyvsp[(5) - (5)].dtype).val);
		 /* This may not be exactly right, but is probably good enough
		  * for the purposes of parsing constant expressions. */
		 (yyval.dtype).type = promote((yyvsp[(3) - (5)].dtype).type, (yyvsp[(5) - (5)].dtype).type);
	       }
    break;

  case 463:

/* Line 1806 of yacc.c  */
#line 6093 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("-%s",(yyvsp[(2) - (2)].dtype).val);
		 (yyval.dtype).type = (yyvsp[(2) - (2)].dtype).type;
	       }
    break;

  case 464:

/* Line 1806 of yacc.c  */
#line 6097 "parser.y"
    {
                 (yyval.dtype).val = NewStringf("+%s",(yyvsp[(2) - (2)].dtype).val);
		 (yyval.dtype).type = (yyvsp[(2) - (2)].dtype).type;
	       }
    break;

  case 465:

/* Line 1806 of yacc.c  */
#line 6101 "parser.y"
    {
		 (yyval.dtype).val = NewStringf("~%s",(yyvsp[(2) - (2)].dtype).val);
		 (yyval.dtype).type = (yyvsp[(2) - (2)].dtype).type;
	       }
    break;

  case 466:

/* Line 1806 of yacc.c  */
#line 6105 "parser.y"
    {
                 (yyval.dtype).val = NewStringf("!%s",(yyvsp[(2) - (2)].dtype).val);
		 (yyval.dtype).type = T_INT;
	       }
    break;

  case 467:

/* Line 1806 of yacc.c  */
#line 6109 "parser.y"
    {
		 String *qty;
                 skip_balanced('(',')');
		 qty = Swig_symbol_type_qualify((yyvsp[(1) - (2)].type),0);
		 if (SwigType_istemplate(qty)) {
		   String *nstr = SwigType_namestr(qty);
		   Delete(qty);
		   qty = nstr;
		 }
		 (yyval.dtype).val = NewStringf("%s%s",qty,scanner_ccode);
		 Clear(scanner_ccode);
		 (yyval.dtype).type = T_INT;
		 Delete(qty);
               }
    break;

  case 468:

/* Line 1806 of yacc.c  */
#line 6125 "parser.y"
    {
	        (yyval.str) = NewString("...");
	      }
    break;

  case 469:

/* Line 1806 of yacc.c  */
#line 6130 "parser.y"
    {
	        (yyval.str) = (yyvsp[(1) - (1)].str);
	      }
    break;

  case 470:

/* Line 1806 of yacc.c  */
#line 6133 "parser.y"
    {
	        (yyval.str) = 0;
	      }
    break;

  case 471:

/* Line 1806 of yacc.c  */
#line 6138 "parser.y"
    {
		 (yyval.bases) = (yyvsp[(1) - (1)].bases);
               }
    break;

  case 472:

/* Line 1806 of yacc.c  */
#line 6143 "parser.y"
    { inherit_list = 1; }
    break;

  case 473:

/* Line 1806 of yacc.c  */
#line 6143 "parser.y"
    { (yyval.bases) = (yyvsp[(3) - (3)].bases); inherit_list = 0; }
    break;

  case 474:

/* Line 1806 of yacc.c  */
#line 6144 "parser.y"
    { (yyval.bases) = 0; }
    break;

  case 475:

/* Line 1806 of yacc.c  */
#line 6147 "parser.y"
    {
		   Hash *list = NewHash();
		   Node *base = (yyvsp[(1) - (1)].node);
		   Node *name = Getattr(base,"name");
		   List *lpublic = NewList();
		   List *lprotected = NewList();
		   List *lprivate = NewList();
		   Setattr(list,"public",lpublic);
		   Setattr(list,"protected",lprotected);
		   Setattr(list,"private",lprivate);
		   Delete(lpublic);
		   Delete(lprotected);
		   Delete(lprivate);
		   Append(Getattr(list,Getattr(base,"access")),name);
	           (yyval.bases) = list;
               }
    break;

  case 476:

/* Line 1806 of yacc.c  */
#line 6164 "parser.y"
    {
		   Hash *list = (yyvsp[(1) - (3)].bases);
		   Node *base = (yyvsp[(3) - (3)].node);
		   Node *name = Getattr(base,"name");
		   Append(Getattr(list,Getattr(base,"access")),name);
                   (yyval.bases) = list;
               }
    break;

  case 477:

/* Line 1806 of yacc.c  */
#line 6173 "parser.y"
    {
		 (yyval.intvalue) = cparse_line;
	       }
    break;

  case 478:

/* Line 1806 of yacc.c  */
#line 6175 "parser.y"
    {
		 (yyval.node) = NewHash();
		 Setfile((yyval.node),cparse_file);
		 Setline((yyval.node),(yyvsp[(2) - (4)].intvalue));
		 Setattr((yyval.node),"name",(yyvsp[(3) - (4)].str));
		 Setfile((yyvsp[(3) - (4)].str),cparse_file);
		 Setline((yyvsp[(3) - (4)].str),(yyvsp[(2) - (4)].intvalue));
                 if (last_cpptype && (Strcmp(last_cpptype,"struct") != 0)) {
		   Setattr((yyval.node),"access","private");
		   Swig_warning(WARN_PARSE_NO_ACCESS, Getfile((yyval.node)), Getline((yyval.node)), "No access specifier given for base class '%s' (ignored).\n", SwigType_namestr((yyvsp[(3) - (4)].str)));
                 } else {
		   Setattr((yyval.node),"access","public");
		 }
		 if ((yyvsp[(4) - (4)].str))
		   SetFlag((yyval.node), "variadic");
               }
    break;

  case 479:

/* Line 1806 of yacc.c  */
#line 6191 "parser.y"
    {
		 (yyval.intvalue) = cparse_line;
	       }
    break;

  case 480:

/* Line 1806 of yacc.c  */
#line 6193 "parser.y"
    {
		 (yyval.node) = NewHash();
		 Setfile((yyval.node),cparse_file);
		 Setline((yyval.node),(yyvsp[(3) - (6)].intvalue));
		 Setattr((yyval.node),"name",(yyvsp[(5) - (6)].str));
		 Setfile((yyvsp[(5) - (6)].str),cparse_file);
		 Setline((yyvsp[(5) - (6)].str),(yyvsp[(3) - (6)].intvalue));
		 Setattr((yyval.node),"access",(yyvsp[(2) - (6)].id));
	         if (Strcmp((yyvsp[(2) - (6)].id),"public") != 0) {
		   Swig_warning(WARN_PARSE_PRIVATE_INHERIT, Getfile((yyval.node)), Getline((yyval.node)), "%s inheritance from base '%s' (ignored).\n", (yyvsp[(2) - (6)].id), SwigType_namestr((yyvsp[(5) - (6)].str)));
		 }
		 if ((yyvsp[(6) - (6)].str))
		   SetFlag((yyval.node), "variadic");
               }
    break;

  case 481:

/* Line 1806 of yacc.c  */
#line 6209 "parser.y"
    { (yyval.id) = (char*)"public"; }
    break;

  case 482:

/* Line 1806 of yacc.c  */
#line 6210 "parser.y"
    { (yyval.id) = (char*)"private"; }
    break;

  case 483:

/* Line 1806 of yacc.c  */
#line 6211 "parser.y"
    { (yyval.id) = (char*)"protected"; }
    break;

  case 484:

/* Line 1806 of yacc.c  */
#line 6215 "parser.y"
    { 
                   (yyval.id) = (char*)"class"; 
		   if (!inherit_list) last_cpptype = (yyval.id);
               }
    break;

  case 485:

/* Line 1806 of yacc.c  */
#line 6219 "parser.y"
    { 
                   (yyval.id) = (char *)"typename"; 
		   if (!inherit_list) last_cpptype = (yyval.id);
               }
    break;

  case 486:

/* Line 1806 of yacc.c  */
#line 6223 "parser.y"
    { 
                   (yyval.id) = (char *)"class..."; 
		   if (!inherit_list) last_cpptype = (yyval.id);
               }
    break;

  case 487:

/* Line 1806 of yacc.c  */
#line 6227 "parser.y"
    { 
                   (yyval.id) = (char *)"typename..."; 
		   if (!inherit_list) last_cpptype = (yyval.id);
               }
    break;

  case 488:

/* Line 1806 of yacc.c  */
#line 6233 "parser.y"
    {
                 (yyval.id) = (yyvsp[(1) - (1)].id);
               }
    break;

  case 489:

/* Line 1806 of yacc.c  */
#line 6236 "parser.y"
    { 
                   (yyval.id) = (char*)"struct"; 
		   if (!inherit_list) last_cpptype = (yyval.id);
               }
    break;

  case 490:

/* Line 1806 of yacc.c  */
#line 6240 "parser.y"
    {
                   (yyval.id) = (char*)"union"; 
		   if (!inherit_list) last_cpptype = (yyval.id);
               }
    break;

  case 493:

/* Line 1806 of yacc.c  */
#line 6250 "parser.y"
    {
                   (yyval.str) = 0;
	       }
    break;

  case 494:

/* Line 1806 of yacc.c  */
#line 6253 "parser.y"
    {
                   (yyval.str) = 0;
	       }
    break;

  case 495:

/* Line 1806 of yacc.c  */
#line 6256 "parser.y"
    {
                   (yyval.str) = 0;
	       }
    break;

  case 496:

/* Line 1806 of yacc.c  */
#line 6259 "parser.y"
    {
                   (yyval.str) = 0;
	       }
    break;

  case 497:

/* Line 1806 of yacc.c  */
#line 6264 "parser.y"
    {
                    (yyval.dtype).throws = (yyvsp[(3) - (4)].pl);
                    (yyval.dtype).throwf = NewString("1");
                    (yyval.dtype).nexcept = 0;
	       }
    break;

  case 498:

/* Line 1806 of yacc.c  */
#line 6269 "parser.y"
    {
                    (yyval.dtype).throws = 0;
                    (yyval.dtype).throwf = 0;
                    (yyval.dtype).nexcept = NewString("true");
	       }
    break;

  case 499:

/* Line 1806 of yacc.c  */
#line 6274 "parser.y"
    {
                    (yyval.dtype).throws = 0;
                    (yyval.dtype).throwf = 0;
                    (yyval.dtype).nexcept = 0;
	       }
    break;

  case 500:

/* Line 1806 of yacc.c  */
#line 6279 "parser.y"
    {
                    (yyval.dtype).throws = 0;
                    (yyval.dtype).throwf = 0;
                    (yyval.dtype).nexcept = NewString("true");
	       }
    break;

  case 501:

/* Line 1806 of yacc.c  */
#line 6284 "parser.y"
    {
                    (yyval.dtype).throws = 0;
                    (yyval.dtype).throwf = 0;
                    (yyval.dtype).nexcept = (yyvsp[(3) - (4)].dtype).val;
	       }
    break;

  case 502:

/* Line 1806 of yacc.c  */
#line 6291 "parser.y"
    {
                    (yyval.dtype).throws = 0;
                    (yyval.dtype).throwf = 0;
                    (yyval.dtype).nexcept = 0;
                    (yyval.dtype).qualifier = (yyvsp[(1) - (1)].str);
               }
    break;

  case 503:

/* Line 1806 of yacc.c  */
#line 6297 "parser.y"
    {
		    (yyval.dtype) = (yyvsp[(1) - (1)].dtype);
                    (yyval.dtype).qualifier = 0;
               }
    break;

  case 504:

/* Line 1806 of yacc.c  */
#line 6301 "parser.y"
    {
		    (yyval.dtype) = (yyvsp[(2) - (2)].dtype);
                    (yyval.dtype).qualifier = (yyvsp[(1) - (2)].str);
               }
    break;

  case 505:

/* Line 1806 of yacc.c  */
#line 6305 "parser.y"
    { 
                    (yyval.dtype).throws = 0;
                    (yyval.dtype).throwf = 0;
                    (yyval.dtype).nexcept = 0;
                    (yyval.dtype).qualifier = 0; 
               }
    break;

  case 506:

/* Line 1806 of yacc.c  */
#line 6313 "parser.y"
    { 
                    Clear(scanner_ccode); 
                    (yyval.decl).have_parms = 0; 
                    (yyval.decl).defarg = 0; 
		    (yyval.decl).throws = (yyvsp[(1) - (3)].dtype).throws;
		    (yyval.decl).throwf = (yyvsp[(1) - (3)].dtype).throwf;
		    (yyval.decl).nexcept = (yyvsp[(1) - (3)].dtype).nexcept;
               }
    break;

  case 507:

/* Line 1806 of yacc.c  */
#line 6321 "parser.y"
    { 
                    skip_balanced('{','}'); 
                    (yyval.decl).have_parms = 0; 
                    (yyval.decl).defarg = 0; 
                    (yyval.decl).throws = (yyvsp[(1) - (3)].dtype).throws;
                    (yyval.decl).throwf = (yyvsp[(1) - (3)].dtype).throwf;
                    (yyval.decl).nexcept = (yyvsp[(1) - (3)].dtype).nexcept;
               }
    break;

  case 508:

/* Line 1806 of yacc.c  */
#line 6329 "parser.y"
    { 
                    Clear(scanner_ccode); 
                    (yyval.decl).parms = (yyvsp[(2) - (4)].pl); 
                    (yyval.decl).have_parms = 1; 
                    (yyval.decl).defarg = 0; 
		    (yyval.decl).throws = 0;
		    (yyval.decl).throwf = 0;
		    (yyval.decl).nexcept = 0;
               }
    break;

  case 509:

/* Line 1806 of yacc.c  */
#line 6338 "parser.y"
    {
                    skip_balanced('{','}'); 
                    (yyval.decl).parms = (yyvsp[(2) - (4)].pl); 
                    (yyval.decl).have_parms = 1; 
                    (yyval.decl).defarg = 0; 
                    (yyval.decl).throws = 0;
                    (yyval.decl).throwf = 0;
                    (yyval.decl).nexcept = 0;
               }
    break;

  case 510:

/* Line 1806 of yacc.c  */
#line 6347 "parser.y"
    { 
                    (yyval.decl).have_parms = 0; 
                    (yyval.decl).defarg = (yyvsp[(2) - (3)].dtype).val; 
                    (yyval.decl).throws = 0;
                    (yyval.decl).throwf = 0;
                    (yyval.decl).nexcept = 0;
               }
    break;

  case 511:

/* Line 1806 of yacc.c  */
#line 6354 "parser.y"
    {
                    (yyval.decl).have_parms = 0;
                    (yyval.decl).defarg = (yyvsp[(3) - (4)].dtype).val;
                    (yyval.decl).throws = (yyvsp[(1) - (4)].dtype).throws;
                    (yyval.decl).throwf = (yyvsp[(1) - (4)].dtype).throwf;
                    (yyval.decl).nexcept = (yyvsp[(1) - (4)].dtype).nexcept;
               }
    break;

  case 518:

/* Line 1806 of yacc.c  */
#line 6373 "parser.y"
    {
		  skip_balanced('(',')');
		  Clear(scanner_ccode);
		}
    break;

  case 519:

/* Line 1806 of yacc.c  */
#line 6385 "parser.y"
    {
		  skip_balanced('{','}');
		  Clear(scanner_ccode);
		}
    break;

  case 520:

/* Line 1806 of yacc.c  */
#line 6391 "parser.y"
    { 
                     String *s = NewStringEmpty();
                     SwigType_add_template(s,(yyvsp[(2) - (3)].p));
                     (yyval.id) = Char(s);
		     scanner_last_id(1);
                 }
    break;

  case 521:

/* Line 1806 of yacc.c  */
#line 6397 "parser.y"
    { (yyval.id) = (char*)"";  }
    break;

  case 522:

/* Line 1806 of yacc.c  */
#line 6401 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].id); }
    break;

  case 523:

/* Line 1806 of yacc.c  */
#line 6402 "parser.y"
    { (yyval.id) = Swig_copy_string("override"); }
    break;

  case 524:

/* Line 1806 of yacc.c  */
#line 6403 "parser.y"
    { (yyval.id) = Swig_copy_string("final"); }
    break;

  case 525:

/* Line 1806 of yacc.c  */
#line 6406 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].id); }
    break;

  case 526:

/* Line 1806 of yacc.c  */
#line 6407 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].dtype).val; }
    break;

  case 527:

/* Line 1806 of yacc.c  */
#line 6408 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].id); }
    break;

  case 528:

/* Line 1806 of yacc.c  */
#line 6411 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].id); }
    break;

  case 529:

/* Line 1806 of yacc.c  */
#line 6412 "parser.y"
    { (yyval.id) = 0; }
    break;

  case 530:

/* Line 1806 of yacc.c  */
#line 6415 "parser.y"
    { 
                  (yyval.str) = 0;
		  if (!(yyval.str)) (yyval.str) = NewStringf("%s%s", (yyvsp[(1) - (2)].str),(yyvsp[(2) - (2)].str));
      	          Delete((yyvsp[(2) - (2)].str));
               }
    break;

  case 531:

/* Line 1806 of yacc.c  */
#line 6420 "parser.y"
    { 
		 (yyval.str) = NewStringf("::%s%s",(yyvsp[(3) - (4)].str),(yyvsp[(4) - (4)].str));
                 Delete((yyvsp[(4) - (4)].str));
               }
    break;

  case 532:

/* Line 1806 of yacc.c  */
#line 6424 "parser.y"
    {
		 (yyval.str) = NewString((yyvsp[(1) - (1)].str));
   	       }
    break;

  case 533:

/* Line 1806 of yacc.c  */
#line 6427 "parser.y"
    {
		 (yyval.str) = NewStringf("::%s",(yyvsp[(3) - (3)].str));
               }
    break;

  case 534:

/* Line 1806 of yacc.c  */
#line 6430 "parser.y"
    {
                 (yyval.str) = NewString((yyvsp[(1) - (1)].str));
	       }
    break;

  case 535:

/* Line 1806 of yacc.c  */
#line 6433 "parser.y"
    {
                 (yyval.str) = NewStringf("::%s",(yyvsp[(3) - (3)].str));
               }
    break;

  case 536:

/* Line 1806 of yacc.c  */
#line 6438 "parser.y"
    {
                   (yyval.str) = NewStringf("::%s%s",(yyvsp[(2) - (3)].str),(yyvsp[(3) - (3)].str));
		   Delete((yyvsp[(3) - (3)].str));
               }
    break;

  case 537:

/* Line 1806 of yacc.c  */
#line 6442 "parser.y"
    {
                   (yyval.str) = NewStringf("::%s",(yyvsp[(2) - (2)].str));
               }
    break;

  case 538:

/* Line 1806 of yacc.c  */
#line 6445 "parser.y"
    {
                   (yyval.str) = NewStringf("::%s",(yyvsp[(2) - (2)].str));
               }
    break;

  case 539:

/* Line 1806 of yacc.c  */
#line 6452 "parser.y"
    {
		 (yyval.str) = NewStringf("::~%s",(yyvsp[(2) - (2)].str));
               }
    break;

  case 540:

/* Line 1806 of yacc.c  */
#line 6458 "parser.y"
    {
                  (yyval.str) = NewStringf("%s%s",(yyvsp[(1) - (2)].id),(yyvsp[(2) - (2)].id));
		  /*		  if (Len($2)) {
		    scanner_last_id(1);
		    } */
              }
    break;

  case 541:

/* Line 1806 of yacc.c  */
#line 6467 "parser.y"
    {
                  (yyval.str) = 0;
		  if (!(yyval.str)) (yyval.str) = NewStringf("%s%s", (yyvsp[(1) - (2)].id),(yyvsp[(2) - (2)].str));
      	          Delete((yyvsp[(2) - (2)].str));
               }
    break;

  case 542:

/* Line 1806 of yacc.c  */
#line 6472 "parser.y"
    {
		 (yyval.str) = NewStringf("::%s%s",(yyvsp[(3) - (4)].id),(yyvsp[(4) - (4)].str));
                 Delete((yyvsp[(4) - (4)].str));
               }
    break;

  case 543:

/* Line 1806 of yacc.c  */
#line 6476 "parser.y"
    {
		 (yyval.str) = NewString((yyvsp[(1) - (1)].id));
   	       }
    break;

  case 544:

/* Line 1806 of yacc.c  */
#line 6479 "parser.y"
    {
		 (yyval.str) = NewStringf("::%s",(yyvsp[(3) - (3)].id));
               }
    break;

  case 545:

/* Line 1806 of yacc.c  */
#line 6482 "parser.y"
    {
                 (yyval.str) = NewString((yyvsp[(1) - (1)].str));
	       }
    break;

  case 546:

/* Line 1806 of yacc.c  */
#line 6485 "parser.y"
    {
                 (yyval.str) = NewStringf("::%s",(yyvsp[(3) - (3)].str));
               }
    break;

  case 547:

/* Line 1806 of yacc.c  */
#line 6490 "parser.y"
    {
                   (yyval.str) = NewStringf("::%s%s",(yyvsp[(2) - (3)].id),(yyvsp[(3) - (3)].str));
		   Delete((yyvsp[(3) - (3)].str));
               }
    break;

  case 548:

/* Line 1806 of yacc.c  */
#line 6494 "parser.y"
    {
                   (yyval.str) = NewStringf("::%s",(yyvsp[(2) - (2)].id));
               }
    break;

  case 549:

/* Line 1806 of yacc.c  */
#line 6497 "parser.y"
    {
                   (yyval.str) = NewStringf("::%s",(yyvsp[(2) - (2)].str));
               }
    break;

  case 550:

/* Line 1806 of yacc.c  */
#line 6500 "parser.y"
    {
		 (yyval.str) = NewStringf("::~%s",(yyvsp[(2) - (2)].id));
               }
    break;

  case 551:

/* Line 1806 of yacc.c  */
#line 6506 "parser.y"
    { 
                   (yyval.id) = (char *) malloc(strlen((yyvsp[(1) - (2)].id))+strlen((yyvsp[(2) - (2)].id))+1);
                   strcpy((yyval.id),(yyvsp[(1) - (2)].id));
                   strcat((yyval.id),(yyvsp[(2) - (2)].id));
               }
    break;

  case 552:

/* Line 1806 of yacc.c  */
#line 6511 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].id);}
    break;

  case 553:

/* Line 1806 of yacc.c  */
#line 6514 "parser.y"
    {
                   (yyval.id) = (char *) malloc(strlen((yyvsp[(1) - (2)].id))+strlen((yyvsp[(2) - (2)].id))+1);
                   strcpy((yyval.id),(yyvsp[(1) - (2)].id));
                   strcat((yyval.id),(yyvsp[(2) - (2)].id));
               }
    break;

  case 554:

/* Line 1806 of yacc.c  */
#line 6526 "parser.y"
    { (yyval.id) = (yyvsp[(1) - (1)].id);}
    break;

  case 555:

/* Line 1806 of yacc.c  */
#line 6529 "parser.y"
    {
		 (yyval.str) = NewString((yyvsp[(1) - (1)].id));
               }
    break;

  case 556:

/* Line 1806 of yacc.c  */
#line 6532 "parser.y"
    {
                  skip_balanced('{','}');
		  (yyval.str) = NewString(scanner_ccode);
               }
    break;

  case 557:

/* Line 1806 of yacc.c  */
#line 6536 "parser.y"
    {
		 (yyval.str) = (yyvsp[(1) - (1)].str);
              }
    break;

  case 558:

/* Line 1806 of yacc.c  */
#line 6541 "parser.y"
    {
                  Hash *n;
                  (yyval.node) = NewHash();
                  n = (yyvsp[(2) - (3)].node);
                  while(n) {
                     String *name, *value;
                     name = Getattr(n,"name");
                     value = Getattr(n,"value");
		     if (!value) value = (String *) "1";
                     Setattr((yyval.node),name, value);
		     n = nextSibling(n);
		  }
               }
    break;

  case 559:

/* Line 1806 of yacc.c  */
#line 6554 "parser.y"
    { (yyval.node) = 0; }
    break;

  case 560:

/* Line 1806 of yacc.c  */
#line 6558 "parser.y"
    {
		 (yyval.node) = NewHash();
		 Setattr((yyval.node),"name",(yyvsp[(1) - (3)].id));
		 Setattr((yyval.node),"value",(yyvsp[(3) - (3)].id));
               }
    break;

  case 561:

/* Line 1806 of yacc.c  */
#line 6563 "parser.y"
    {
		 (yyval.node) = NewHash();
		 Setattr((yyval.node),"name",(yyvsp[(1) - (5)].id));
		 Setattr((yyval.node),"value",(yyvsp[(3) - (5)].id));
		 set_nextSibling((yyval.node),(yyvsp[(5) - (5)].node));
               }
    break;

  case 562:

/* Line 1806 of yacc.c  */
#line 6569 "parser.y"
    {
                 (yyval.node) = NewHash();
                 Setattr((yyval.node),"name",(yyvsp[(1) - (1)].id));
	       }
    break;

  case 563:

/* Line 1806 of yacc.c  */
#line 6573 "parser.y"
    {
                 (yyval.node) = NewHash();
                 Setattr((yyval.node),"name",(yyvsp[(1) - (3)].id));
                 set_nextSibling((yyval.node),(yyvsp[(3) - (3)].node));
               }
    break;

  case 564:

/* Line 1806 of yacc.c  */
#line 6578 "parser.y"
    {
                 (yyval.node) = (yyvsp[(3) - (3)].node);
		 Setattr((yyval.node),"name",(yyvsp[(1) - (3)].id));
               }
    break;

  case 565:

/* Line 1806 of yacc.c  */
#line 6582 "parser.y"
    {
                 (yyval.node) = (yyvsp[(3) - (5)].node);
		 Setattr((yyval.node),"name",(yyvsp[(1) - (5)].id));
		 set_nextSibling((yyval.node),(yyvsp[(5) - (5)].node));
               }
    break;

  case 566:

/* Line 1806 of yacc.c  */
#line 6589 "parser.y"
    {
		 (yyval.id) = (yyvsp[(1) - (1)].id);
               }
    break;

  case 567:

/* Line 1806 of yacc.c  */
#line 6592 "parser.y"
    {
                 (yyval.id) = Char((yyvsp[(1) - (1)].dtype).val);
               }
    break;



/* Line 1806 of yacc.c  */
#line 12672 "y.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 6599 "parser.y"


SwigType *Swig_cparse_type(String *s) {
   String *ns;
   ns = NewStringf("%s;",s);
   Seek(ns,0,SEEK_SET);
   scanner_file(ns);
   top = 0;
   scanner_next_token(PARSETYPE);
   yyparse();
   /*   Printf(stdout,"typeparse: '%s' ---> '%s'\n", s, top); */
   return top;
}


Parm *Swig_cparse_parm(String *s) {
   String *ns;
   ns = NewStringf("%s;",s);
   Seek(ns,0,SEEK_SET);
   scanner_file(ns);
   top = 0;
   scanner_next_token(PARSEPARM);
   yyparse();
   /*   Printf(stdout,"typeparse: '%s' ---> '%s'\n", s, top); */
   Delete(ns);
   return top;
}


ParmList *Swig_cparse_parms(String *s, Node *file_line_node) {
   String *ns;
   char *cs = Char(s);
   if (cs && cs[0] != '(') {
     ns = NewStringf("(%s);",s);
   } else {
     ns = NewStringf("%s;",s);
   }
   Setfile(ns, Getfile(file_line_node));
   Setline(ns, Getline(file_line_node));
   Seek(ns,0,SEEK_SET);
   scanner_file(ns);
   top = 0;
   scanner_next_token(PARSEPARMS);
   yyparse();
   /*   Printf(stdout,"typeparse: '%s' ---> '%s'\n", s, top); */
   return top;
}


