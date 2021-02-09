% Copy to ~/.config/mc/plugin for this file to be loaded at mc's startup.

% Implement left word capitalizing.
define capitalize() {
    variable word;
    % Get the word left of cursor.
    word = mc->cure_get_left_whole_word(0);
    % Move to the beginning of it.
    mc->cure_cursor_move(-strlen(word));
    % Replace the char with capitalized version of it.
    mc->cure_delete();
    mc->cure_insert_ahead(toupper(word[0]));
}

% Map the function capitalize() to the Alt-Shift-C key.
mc->editor_map_key_to_func("Capitalize","alt-shift-c","capitalize");

% Implement left word up-casing.
define toupper_word() {
    variable word;
    % Get the word left of cursor.
    word = mc->cure_get_left_whole_word(0);
    % Move to the beginning of it.
    % mc->cure_cursor_move(-strlen(word));
    % Replace the word with up-cased version of it.
    variable i, len = strlen(word);
    for (i=0; i<len; i++)
    {
        mc->cure_backspace();
        mc->cure_insert_ahead(toupper(word[len-i-1]));
    }
}

% Map the function toupper_word() to the Alt-Ctrl-c key.
mc->editor_map_key_to_func("ToUpper","alt-ctrl-u","toupper_word");
