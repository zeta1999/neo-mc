% MCEdit Startup Script in S-Lang Scripting Language
%
% See:
% – https://www.jedsoft.org/slang/doc/html/slang.html,
% for a reference on the language.

% Increase to 2 to have confirmation messages after loading this script and other plugins.
%mc_loglevel=3;

% A function showing a listbox:
define listbox_display_function() {
    variable items = ["This is a listbox", "It's displayed from…",
        "…`init.sl` S-Lang startup script"];

    variable sel = mc->listbox(5, 35, "Welcome",items);

    if (sel >= 0)
        mc->message("You have selected:", "item #" + string(sel+1) + ": " + items[sel]);
    else
        mc->message("Info", "No selection have been made");

    return 1;
}

define action_funct() {
    variable ret;
    ret = mc->action("GrowInteger");
    ret += 10*mc->action("Up");
    ret += 100*mc->action(3);
    mc->message("GrowInteger and 2x Up, return values:", string(ret));
    return ret;
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
