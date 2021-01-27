% Copy to ~/.config/mc/plugin for this file to be loaded at mc's startup.

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

% Map the function capitalize() to the Alt-c key.
mc->editor_map_key_to_func("Capitalize","alt-c","capitalize");

