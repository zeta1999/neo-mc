% MCEdit Startup Script in S-Lang Scripting Language
%
% See:
% – https://www.jedsoft.org/slang/doc/html/slang.html,
% for a reference on the language.

% Increase to 2 to have confirmation messages after loading this script and other plugins.
%mc_loglevel=3;

define util_get_value_field(s) {
    return s.value;
}
% A function showing a listbox:
define listbox_display_function() {
    variable items, items2, i, count;

    % Show all functions in the `mc` namespace.
    items = _apropos("mc",".*",0xF);
    i = array_sort(items);
    items = items[i];
    variable sel = mc->listbox(15, 35, "Welcome! S-Lang API (exported) functions:", items);

    if (sel >= 0)
        mc->message("You have selected:", "item #" + string(sel+1) + ": " + items[sel]);
    else
        mc->message("Info", "No selection have been made");

    % Show all preprocessor symbols that exist.
    count = __get_defined_symbols();
    items = __pop_args (count);
    items2 = array_map(String_Type, &util_get_value_field, items);

    sel = mc->listbox(15, 55, "S-Lang interpreter defined macros:", items2);

    if (sel >= 0)
        mc->message("You have selected:", "item #" + string(sel+1) + ": " + items2[sel]);
    else
        mc->message("Info", "No selection have been made");

    return 1;
}

define action_funct() {
    variable ret=0;
    % ret = mc->action("GrowInteger");
    ret += 100*mc->action("Cut";param=33);
    ret += 100*mc->action("Cut";data="/home/guest/zshrc2");
    ret += 100*mc->action("EditFile";data="/home/guest/zshrc2");
    % mc->message("GrowInteger and 2x Up, return values:", string(ret));
    return 1;
}

% A function causing runtime error:
define divide_by_zero() {
    variable string = "Divided by 0 on purpose";
    variable mc = 1 / 0;
}

% A function to present backtrace functionality:
define b_function() {
    variable tmp_variable = 5;
    divide_by_zero();
    return 0;
}

% A function to present backtrace functionality:
define a_function() {
    b_function();
    return 0;
}


% Show an error message on Ctrl-t:
mc->editor_map_key_to_func("DivBy0Action", "ctrl-t", "a_function");

% Show a listbox on Alt-y:
mc->editor_map_key_to_func("ListboxAction", "alt-y", "listbox_display_function");
mc->editor_map_key_to_func("ActionFunct", "ctrl-alt-y", "action_funct");
