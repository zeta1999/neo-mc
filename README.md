# ⁝⁝⁝ ⋱הϵѻ⋱ Midnight Commander ⁝⁝⁝

Welcome to the ⋱Neo⋱-MC project! The goals of it are to:

- make the hidden gem – mcedit – shine and grow to be able to compete with Vim and Emacs,
- add a scripting language to mcedit and mc to make this possible,
- add some meaningful plugins written in the scripting language.

Check out [MCEditWishList](https://github.com/neo-mc/neo-mc/wiki/MCEditWishList) for a curated list
of the planned enhancements. Maybe you would want to implement one of them? Because patches are
welcomed, so as are new contributors.

# ⋱ New Features Added By The Fork ⋱

## Multi Search – AND-chained Grepping (Filtering) Of Any Listbox

On Ctrl-Space an input field for a search query will be added **under the currently displayed 
listbox**. Entering any text into it will cause **only the matching** (i.e.: containing) elements in
the listbox to be displayed – the rest will be removed. The query can be multi word (all must match).
Also, entering just the letter '**c**' or '**h**' will automatically filter entries that end on
'**.c**' or '**.h**', respectively (it also looks for .cpp and .hpp extensions).

MultiSearch can be enabled by default on all listboxes via the option
`multi_search_active_by_default=true`.

## Listing Of TAGS Symbols

A feature that pairs up with MultiSearch – an ability to **list** of all ctags symbols for the
current file:

- to list all functions – press alt-shift-f,
- … all variables defined in the file – press alt-shift-v,
- … all type definitions … – press alt-shift-t,
- … all remaining types of entities (e.g.: C macros) … – press alt-shift-o,
- … all symbols regardless of their type – press alt-shift-a.

After selecting of an entry the cursor and display will jump to it. With MultiSearch you can quickly
find the declaration that you want by grepping the list.

## Completion From All Open Files

The completion has been extended to propose words from **all open files**, not only from the
currently edited one.

## Viewport Centering

A new action called `CenterView` has been added. By pressing Alt-c the currently edited line of text
will be positioned in the middle of the screen (i.e.: the viewport will be scrolled).

## Window Cascading And Tiling

When Ctrl-Alt-c will be pressed then all windows with opened files will exit fullscreen and be 
automatically arranged in a cascade. Ctrl-Alt-t arranges them in a tiling configuration.

Also, new options `-w/--cascade` and `-T,--tile` will make the editor to start up with the
selected arrangement.







