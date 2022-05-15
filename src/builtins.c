/* Builtin variables */
{.name="true",                .type=var, .rets=true, .ret=boolean},
{.name="false",               .type=var, .rets=true, .ret=boolean},

/* Builtin functions */
{.name="+",                   .type=func, .argc=2, .args={number, number},      .rets=true, .ret=number},
{.name="-",                   .type=func, .argc=2, .args={number, number},      .rets=true, .ret=number},
{.name="*",                   .type=func, .argc=2, .args={number, number},      .rets=true, .ret=number},
{.name="/",                   .type=func, .argc=2, .args={number, number},      .rets=true, .ret=number},
{.name=">",                   .type=func, .argc=2, .args={number, number},      .rets=true, .ret=boolean},
{.name=">=",                  .type=func, .argc=2, .args={number, number},      .rets=true, .ret=boolean},
{.name="<",                   .type=func, .argc=2, .args={number, number},      .rets=true, .ret=boolean},
{.name="<=",                  .type=func, .argc=2, .args={number, number},      .rets=true, .ret=boolean},
{.name="==",                  .type=func, .argc=2, .args={any, any},            .rets=true, .ret=boolean},
{.name="/=",                  .type=func, .argc=2, .args={any, any},            .rets=true, .ret=boolean},
{.name="match",               .type=func, .argc=2, .args={any, any},            .rets=true, .ret=boolean},
{.name="if",                  .type=func, .argc=3, .args={boolean, any, any},   .rets=true, .ret=any},
{.name="when",                .type=func, .argc=2, .args={boolean, block},      .rets=false, .needs_stack=true},
{.name="unless",              .type=func, .argc=2, .args={boolean, block},      .rets=false, .needs_stack=true},
{.name="while",               .type=func, .argc=2, .args={block, block},        .needs_stack=true},
{.name="print",               .type=func, .argc=1, .args={any}},
{.name="put",                 .type=func, .argc=1, .args={any}},
{.name="prints",              .type=func, .argc=1, .args={block}},
{.name="puts",                .type=func, .argc=1, .args={block}},
{.name="do",                  .type=func, .argc=1, .args={block}, .needs_stack=true},
{.name="random",              .type=func, .argc=3, .args={number, number},   .rets=true, .ret=number},
{.name="modulo",              .type=func, .argc=2, .args={number, number},   .rets=true, .ret=number},
{.name="sqrt",                .type=func, .argc=1, .args={number},           .rets=true, .ret=number},
{.name="either",              .type=func, .argc=2, .args={boolean, boolean}, .rets=true, .ret=boolean},
{.name="both",                .type=func, .argc=2, .args={boolean, boolean}, .rets=true, .ret=boolean},
{.name="one-of",              .type=func, .argc=2, .args={boolean, boolean}, .rets=true, .ret=boolean},
{.name="not",                 .type=func, .argc=1, .args={boolean},          .rets=true, .ret=boolean},
{.name="number?",             .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="symbol?",             .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="integer?",            .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="zero?",               .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="any?",                .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="list?",               .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="string?",             .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="block?",              .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="boolean?",            .type=func, .argc=1, .args={any},       .rets=true, .ret=boolean},
{.name="first",               .type=func, .argc=1, .args={list},      .rets=true, .ret=any},
{.name="rest",                .type=func, .argc=1, .args={list},      .rets=true, .ret=list},
{.name="head",                .type=func, .argc=1, .args={string},    .rets=true, .ret=string},
{.name="tail",                .type=func, .argc=1, .args={string},    .rets=true, .ret=string},
{.name="push",                .type=func, .argc=2, .args={any, list}, .rets=true, .ret=list},
{.name="append",              .type=func, .argc=2, .args={any, list}, .rets=true, .ret=list},
{.name="empty?",              .type=func, .argc=1, .args={list},      .rets=true, .ret=boolean},
{.name="join",                .type=func, .argc=1, .args={number},    .rets=true, .ret=string, .needs_stack=true},
{.name="string-length",       .type=func, .argc=1, .args={string},    .rets=true, .ret=number},
{.name="substring",           .type=func, .argc=3, .args={number, number, string}, .rets=true, .ret=string},
{.name="match-regex",         .type=func, .argc=2, .args={string, string},         .rets=true, .ret=boolean},
{.name="ordinal",             .type=func, .argc=1, .args={string}, .rets=true, .ret=number},
{.name="character",           .type=func, .argc=1, .args={number}, .rets=true, .ret=string},
{.name="split",               .type=func, .argc=2, .args={string, string}, .rets=true, .ret=list},
{.name="floor",               .type=func, .argc=1, .args={number}, .rets=true, .ret=number},
{.name="round",               .type=func, .argc=1, .args={number}, .rets=true, .ret=number},
{.name="ceiling",             .type=func, .argc=1, .args={number}, .rets=true, .ret=number},
{.name="assert",              .type=func, .argc=2, .args={string,boolean}},
{.name="error",               .type=func, .argc=1, .args={string}},
{.name="list",                .type=func, .argc=1, .args={block},       .rets=true, .ret=list},
{.name="number",              .type=func, .argc=1, .args={string},      .rets=true, .ret=number},
{.name="map",                 .type=func, .argc=2, .args={block, list}, .rets=true, .ret=list},
{.name="filter",              .type=func, .argc=2, .args={block, list}, .rets=true, .ret=list, .needs_stack=true},
{.name="for",                 .type=func, .argc=2, .args={list, block}, .rets=false, .needs_stack=true},
{.name="range",               .type=func, .argc=2, .args={number, number},  .rets=true, .ret=list},
{.name="index",               .type=func, .argc=2, .args={number, list},            .rets=true, .ret=any},
{.name="precompute",          .type=func, .argc=1, .args={block},   .rets=true, .ret=block},
{.name="wait",                .type=func, .argc=1, .args={number},  .rets=false},
{.name="stop",                .type=func, .argc=0, .rets=false},
{.name="show",                .type=func, .argc=1, .args={any},     .rets=true, .ret=string},
{.name="case",                .type=func, .argc=3, .args={any, any, any}, .rets=true, .ret=block},
{.name="stack",               .type=func, .rets=true, .ret=list, .needs_stack=true},
{.name="clear",               .type=func, .argc=0, .needs_stack=true},
{.name="length",              .type=func, .argc=1, .args={list},    .rets=true, .ret=number},
{.name="take",                .type=func, .argc=2, .args={number, list}, .rets=true, .ret=list},
{.name="take-while",          .type=func, .argc=2, .args={block, list}, .rets=true, .ret=list},
{.name="discard",             .type=func, .argc=2, .args={number, list}, .rets=true, .ret=list},
{.name="remember",            .type=func, .argc=1, .args={block},        .rets=true, .ret=block},
{.name="pure",                .type=func, .argc=1, .args={block},        .rets=true, .ret=block},
{.name="all",                 .type=func, .argc=2, .args={block, list}, .rets=true, .ret=boolean},

/* Builtin stack operations */
{.name="drop",                .type=stack_op, .stack_shuffle=&drop_register},
{.name="twin",                .type=stack_op, .stack_shuffle=&twin_register},
{.name="triplet",             .type=stack_op, .stack_shuffle=&triplet_register},
{.name="swap",                .type=stack_op, .stack_shuffle=&swap_register},

/* Builtin IO */

#ifndef DISABLEIO
{.name="parameters",          .type=func, .rets=true, .ret=list},
{.name="input",               .type=func, .rets=true, .ret=string},
{.name="read",                .type=func, .argc=1, .args={string}, .rets=true, .ret=string},
{.name="write",               .type=func, .argc=2, .args={string,any}},
{.name="path",                .type=func, .rets=true, .ret=string},
#endif
