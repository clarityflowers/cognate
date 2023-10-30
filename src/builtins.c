/* Builtin functions */
{.name="empty",               .calltype=call, .argc=0, .returns=true, .rettype=list},
{.name="true",                .calltype=call, .argc=0, .returns=true, .rettype=boolean},
{.name="false",               .calltype=call, .argc=0, .returns=true, .rettype=boolean},
{.name="+",                   .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=number},
{.name="-",                   .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=number},
{.name="*",                   .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=number},
{.name="/",                   .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=number},
{.name=">",                   .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=boolean},
{.name=">=",                  .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=boolean},
{.name="<",                   .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=boolean},
{.name="<=",                  .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=boolean},
{.name="==",                  .calltype=call, .argc=2, .args={any, any},            .returns=true, .rettype=boolean},
{.name="!=",                  .calltype=call, .argc=2, .args={any, any},            .returns=true, .rettype=boolean},
{.name="^",                   .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=number},
{.name="match",               .calltype=call, .argc=2, .args={any, any},            .returns=true, .rettype=boolean},
{.name="if",                  .calltype=branch, .argc=3, .args={boolean, any, any},   .returns=true, .rettype=any},
{.name="print",               .calltype=call, .argc=1, .args={any}},
{.name="put",                 .calltype=call, .argc=1, .args={any}},
{.name="prints",              .calltype=call, .argc=1, .args={block}},
{.name="puts",                .calltype=call, .argc=1, .args={block}},
{.name="do",                  .calltype=call, .argc=1, .args={block}, .stack=true},
{.name="random",              .calltype=call, .argc=2, .args={number, number},   .returns=true, .rettype=number},
{.name="modulo",              .calltype=call, .argc=2, .args={number, number},   .returns=true, .rettype=number},
{.name="sqrt",                .calltype=call, .argc=1, .args={number},           .returns=true, .rettype=number},
{.name="or",                  .calltype=call, .argc=2, .args={boolean, boolean}, .returns=true, .rettype=boolean},
{.name="and",                 .calltype=call, .argc=2, .args={boolean, boolean}, .returns=true, .rettype=boolean},
{.name="xor",                 .calltype=call, .argc=2, .args={boolean, boolean}, .returns=true, .rettype=boolean},
{.name="not",                 .calltype=call, .argc=1, .args={boolean},          .returns=true, .rettype=boolean},
{.name="number?",             .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean, /*.checks=number*/},
{.name="io?",                 .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean, /*.checks=io*/},
{.name="symbol?",             .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean, /*.checks=symbol*/},
{.name="integer?",            .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean},
{.name="zero?",               .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean},
{.name="any?",                .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean},
{.name="list?",               .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean, /*.checks=list*/},
{.name="string?",             .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean, /*.checks=string*/},
{.name="block?",              .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean, /*.checks=block*/},
{.name="boolean?",            .calltype=call, .argc=1, .args={any},       .returns=true, .rettype=boolean, /*.checks=boolean*/},
{.name="number!",             .calltype=call, .argc=1, .args={number}, .returns=true, .rettype=number},
{.name="symbol!",             .calltype=call, .argc=1, .args={symbol}, .returns=true, .rettype=symbol},
{.name="io!",                 .calltype=call, .argc=1, .args={io}, .returns=true, .rettype=io},
{.name="integer!",            .calltype=call, .argc=1, .args={number}, .returns=true, .rettype=number},
{.name="zero!",               .calltype=call, .argc=1, .args={number}, .returns=true, .rettype=number},
{.name="any!",                .calltype=call, .argc=1, .args={any},    .returns=true, .rettype=any},
{.name="list!",               .calltype=call, .argc=1, .args={list},   .returns=true, .rettype=list},
{.name="string!",             .calltype=call, .argc=1, .args={string}, .returns=true, .rettype=string},
{.name="block!",              .calltype=call, .argc=1, .args={block},  .returns=true, .rettype=block},
{.name="boolean!",            .calltype=call, .argc=1, .args={boolean},.returns=true, .rettype=boolean},

{.name="first",               .calltype=call, .argc=1, .args={list},      .returns=true, .rettype=any},
{.name="rest",                .calltype=call, .argc=1, .args={list},      .returns=true, .rettype=list},
{.name="head",                .calltype=call, .argc=1, .args={string},    .returns=true, .rettype=string},
{.name="tail",                .calltype=call, .argc=1, .args={string},    .returns=true, .rettype=string},
{.name="push",                .calltype=call, .argc=2, .args={any, list}, .returns=true, .rettype=list},
{.name="empty?",              .calltype=call, .argc=1, .args={list},      .returns=true, .rettype=boolean},
{.name="join",                .calltype=call, .argc=2, .args={string, string}, .returns=true, .rettype=string},
{.name="string-length",       .calltype=call, .argc=1, .args={string},    .returns=true, .rettype=number},
{.name="substring",           .calltype=call, .argc=3, .args={number, number, string}, .returns=true, .rettype=string},
//{.name="regex",               .calltype=call, .argc=1, .args={string},    .returns=true, .rettype=block},
{.name="ordinal",             .calltype=call, .argc=1, .args={string}, .returns=true, .rettype=number},
{.name="character",           .calltype=call, .argc=1, .args={number}, .returns=true, .rettype=string},
{.name="split",               .calltype=call, .argc=2, .args={string, string}, .returns=true, .rettype=list},
{.name="floor",               .calltype=call, .argc=1, .args={number}, .returns=true, .rettype=number},
{.name="round",               .calltype=call, .argc=1, .args={number}, .returns=true, .rettype=number},
{.name="ceiling",             .calltype=call, .argc=1, .args={number}, .returns=true, .rettype=number},
{.name="assert",              .calltype=call, .argc=2, .args={string,boolean}},
{.name="error",               .calltype=call, .argc=1, .args={string}},
{.name="list",                .calltype=call, .argc=1, .args={block},       .returns=true, .rettype=list},
{.name="number",              .calltype=call, .argc=1, .args={string},      .returns=true, .rettype=number},
{.name="map",                 .calltype=call, .argc=2, .args={block, list}, .returns=true, .rettype=list, .stack=true},
{.name="range",               .calltype=call, .argc=2, .args={number, number},  .returns=true, .rettype=list},
{.name="index",               .calltype=call, .argc=2, .args={number, list},            .returns=true, .rettype=any},
{.name="precompute",          .calltype=call, .argc=1, .args={block},   .returns=true, .rettype=block},
{.name="wait",                .calltype=call, .argc=1, .args={number},  .returns=false},
{.name="stop",                .calltype=call, .argc=0, .returns=false},
{.name="show",                .calltype=call, .argc=1, .args={any},     .returns=true, .rettype=string},
{.name="stack",               .calltype=call, .returns=true, .rettype=list, .stack=true},
{.name="clear",               .calltype=call, .argc=0, .stack=true},
{.name="length",              .calltype=call, .argc=1, .args={list},    .returns=true, .rettype=number},
{.name="take",                .calltype=call, .argc=2, .args={number, list}, .returns=true, .rettype=list},
{.name="discard",             .calltype=call, .argc=2, .args={number, list}, .returns=true, .rettype=list},
{.name="remember",            .calltype=call, .argc=1, .args={block},        .returns=true, .rettype=block},
{.name="pure",                .calltype=call, .argc=1, .args={block},        .returns=true, .rettype=block},
{.name="box",                 .calltype=call, .argc=1, .args={any}, .returns=true, .rettype=box},
{.name="unbox",               .calltype=call, .argc=1, .args={box}, .returns=true, .rettype=any},
{.name="set",                 .calltype=call, .argc=2, .args={box, any}, .returns=false},
{.name="debug",               .calltype=call, .argc=0, .returns=false},
{.name="begin",               .calltype=call, .argc=1, .args={block}, .stack=true},

/* math */
{.name="sind",                   .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="cosd",                   .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="tand",                   .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},

{.name="sin",                   .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="cos",                   .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="tan",                   .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},

{.name="exp",                    .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="log",                    .calltype=call, .argc=2, .args={number, number},      .returns=true, .rettype=number},
{.name="ln",                     .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},

{.name="asind",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="acosd",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="atand",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},

{.name="asin",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="acos",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="atan",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},

{.name="sinhd",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="coshd",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="tanhd",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},

{.name="sinh",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="cosh",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},
{.name="tanh",                  .calltype=call, .argc=1, .args={number},      .returns=true, .rettype=number},



/* Builtin stack operations */
//{.name="drop",                .calltype=stack_op, .stack_shuffle=&drop_register},
//{.name="twin",                .calltype=stack_op, .stack_shuffle=&twin_register},
//{.name="triplet",             .calltype=stack_op, .stack_shuffle=&triplet_register},
//{.name="swap",                .calltype=stack_op, .stack_shuffle=&swap_register},

/* Builtin IO */

#ifndef DISABLEIO
{.name="parameters",          .calltype=call, .returns=true, .rettype=list},
{.name="input",               .calltype=call, .returns=true, .rettype=string},
{.name="open",                .calltype=call, .argc=2, .args={string, string}, .returns=true, .rettype=io},
{.name="with",                .calltype=call, .argc=3, .args={string, string, block}, .stack=true},
{.name="read-file",           .calltype=call, .argc=1, .args={io}, .returns=true, .rettype=string},
{.name="close",               .calltype=call, .argc=1, .args={io}},
{.name="path",                .calltype=call, .returns=true, .rettype=string},
#endif

{.name=NULL},
