let g:netrw_liststyle = 3
let g:netrw_browse_split = 0

let g:ctrlp_extensions = ['tag', 'buffertag'] 
let g:ale_linters = {'python': ['pyls', 'mypy']}

let g:ale_python_pyls_config = {
	\   'pyls': {
		\    'plugins': {
			\     'pylint': {
				\       'enabled' : v:true 
					\     },
			\     'pyflakes': {
				\       'enabled': v:true
					\     },
			\     'pydocstyle': {
				\      'enabled': v:true
					\     },
			\     'pycodestyle': {
				\      'enabled': v:true
					\     }
			\   }
		\ }
	\}



set nocompatible

set autoread
set autoindent
set completeopt-=preview
set encoding=utf-8
set cursorline
set hidden
set history=50
set incsearch
set laststatus=2
set nowrap
set number
set relativenumber
set ruler
set showcmd
set smartcase
set visualbell
set wildmenu
set wildignore=*.pyc,
set colorcolumn=80
set scrolloff=4

" Disable execution of modeline
set nomodeline
set nomodelineexpr

" Gvim
set guioptions-=r
set guioptions-=L
set guioptions+=m
set guioptions-=T


" Search
set hlsearch
set ignorecase
" USE ENGLISH
set langmenu=en_US
let $LANG = 'en_US'
"-----------------
" Set Path
if has("win32")
   let $PATH = 'C:\msys64\usr\bin;' . $PATH
   set viewdir=~\vimfiles\views
   set dir=~\vimfiles\swap
endif
set path=,.,,**


"  MAPPINGS
map Y y$
" map <C-K> :bprev<CR>
" map <C-J> :bnext<CR>

let mapleader="\<Space>"

" For leader key
noremap <Leader>y "+y
noremap <Leader>d "+d
nnoremap <Leader>p "+p
nnoremap <Leader>P "+P
noremap <Leader>p "+p
noremap <Leader>P "+P
noremap <Leader>n :nohl<CR>
noremap <Leader>c :close<CR>
noremap <Leader>o :only<CR>
noremap <Leader>b :bd<CR>
nnoremap <Leader>s :%s/\<<C-r><C-w>\>//g<Left><Left>

noremap <Leader><C-r> :CtrlPBufTag<CR>
noremap <Leader><C-p> :CtrlPTag<CR>
inoremap <silent> <C-Space> <C-\><C-O>:ALEComplete<CR>


"-----------------
set backspace=indent,eol,start

filetype on
syntax on

set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab

function! <SID>StripTrailingWhitespaces()
    " Preparation: save last search, and cursor position.
    let _s=@/
    let l = line(".")
    let c = col(".")
    " Do the business:
    %s/\s\+$//e
    " Clean up: restore previous search history, and cursor position
    let @/=_s
    call cursor(l, c)
endfunction

if has("autocmd")
	augroup rc
		autocmd FileType python setlocal tabstop=4 softtabstop=4 shiftwidth=4 expandtab
		autocmd BufNewFile,BufRead *.m setfiletype matlab
		autocmd BufWinLeave *.* mkview
		autocmd BufWinEnter *.* silent loadview
	augroup END
endif

colorscheme desert

" Mappings
inoremap jj <Esc>
inoremap jk <Esc>
inoremap <esc> <nop>

vmap X y/<C-R>"<CR>

cd ~
" set makeprg=python\ -m\ unittest\ discover\ -s\ src\ -p\ \"*_test.py\"\ -v\ 2>&1
" set guifont=Consolas:h12


set laststatus=2
set statusline=
set statusline+=%F
set statusline+=%=
set statusline+=%m
set statusline+=%h
set statusline+=%r
set statusline+=\ 
set statusline+=%l
set statusline+=/
set statusline+=%L
set statusline+=\ 
set statusline+=<
set statusline+=%c
set statusline+=>
set statusline+=\ 
set statusline+=%y
set statusline+=\ 
set statusline+=%{&ff}
set statusline+=\ 
set statusline+=%{strlen(&fenc)?&fenc:'none'}
set statusline+=\ 
set statusline+=%k
set statusline+=\ 
set statusline+=%P
