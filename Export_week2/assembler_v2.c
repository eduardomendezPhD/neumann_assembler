// assembler_v2.c  -- two-pass assembler for tiny ISA (LOAD, ADD, STORE, JMP, JZ, HALT)
// Usage: ./assembler_v2 input.asm output_base
// Produces: output_base.mem (text hex, 1 byte/line)
//           output_base.bin (raw bytes)
//           output_base.lst (detailed listing with symbol table)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

#define MEM_SIZE     256
#define MAX_LINE     512
#define MAX_TOKS     16
#define MAX_SYMBOLS  1024
#define MAX_LINES    2000

typedef struct { char name[64]; int value; } Symbol;
static Symbol symtab[MAX_SYMBOLS];
static int symcount = 0;

typedef struct {
    int addr;          // dirección donde comienza la emisión de esta línea
    int nbytes;        // bytes emitidos (0,1,2)
    uint8_t bytes[2];  // hasta 2 bytes (nuestra ISA actual)
    char source[MAX_LINE]; // línea fuente (limpia)
} Listing;

static Listing listing[MAX_LINES];
static int list_count = 0;

static uint8_t out_mem[MEM_SIZE];
static int used[MEM_SIZE]; // 0/1

// Opcodes
enum {
    OP_LOAD  = 0x01,
    OP_ADD   = 0x02,
    OP_STORE = 0x03,
    OP_JMP   = 0x04,
    OP_JZ    = 0x05,
    OP_HALT  = 0xFF
};

static void die(const char *msg) { fprintf(stderr, "ERROR: %s\n", msg); exit(1); }

// trims
static char *ltrim(char *s){ while(isspace((unsigned char)*s)) s++; return s;}
static void rtrim_inplace(char *s){ int i = (int)strlen(s)-1; while(i>=0 && isspace((unsigned char)s[i])) s[i--]=0; }
static void strtolower(char *dst, const char *src){ while(*src){ *dst++ = (char)tolower((unsigned char)*src++); } *dst=0; }

static int find_symbol(const char *name){
    for(int i=0;i<symcount;i++) if(strcmp(symtab[i].name, name)==0) return i;
    return -1;
}
static void add_symbol(const char *name, int value){
    if(find_symbol(name) != -1){ fprintf(stderr,"Symbol redefinition: %s\n", name); exit(1); }
    if(symcount >= MAX_SYMBOLS) die("symbol table full");
    strncpy(symtab[symcount].name, name, sizeof(symtab[0].name)-1);
    symtab[symcount].name[sizeof(symtab[0].name)-1] = 0;
    symtab[symcount].value = value & 0xFF;
    symcount++;
}

static int parse_number(const char *tok, int *ok){
    *ok = 1;
    if(tok[0]=='0' && tok[1]=='x'){ return (int)strtol(tok+2, NULL, 16); }
    if(tok[0]=='0' && tok[1]=='b'){ return (int)strtol(tok+2, NULL, 2); }
    if(isdigit((unsigned char)tok[0]) || (tok[0]=='-' && isdigit((unsigned char)tok[1]))){
        return (int)strtol(tok, NULL, 10);
    }
    *ok = 0; return 0;
}

static void tokenize(char *line, char **toks, int *nt){
    *nt = 0;
    char *p = line;
    while(*p){
        while(isspace((unsigned char)*p)) p++;
        if(*p==0 || *p==';') break;
        char *start = p;
        // token simple hasta espacio o ';'
        while(*p && !isspace((unsigned char)*p) && *p!=';') p++;
        int len = (int)(p - start);
        if(len>0){
            char *tok = (char*)malloc(len+1);
            memcpy(tok, start, len);
            tok[len]=0;
            toks[(*nt)++] = tok;
        }
        if(*p==';') break;
    }
}

static void free_toks(char **toks, int nt){ for(int i=0;i<nt;i++) free(toks[i]); }

static int resolve_operand(const char *op, int *ok){
    *ok = 1;
    int numok=0; int val = parse_number(op, &numok);
    if(numok) return val & 0xFF;
    int idx = find_symbol(op);
    if(idx>=0) return symtab[idx].value & 0xFF;
    *ok = 0; return 0;
}

static void add_listing(int addr, int nbytes, uint8_t b0, uint8_t b1, const char *clean_src){
    if(list_count >= MAX_LINES) return;
    listing[list_count].addr = addr;
    listing[list_count].nbytes = nbytes;
    listing[list_count].bytes[0] = b0;
    listing[list_count].bytes[1] = b1;
    strncpy(listing[list_count].source, clean_src, sizeof(listing[list_count].source)-1);
    listing[list_count].source[sizeof(listing[list_count].source)-1] = 0;
    list_count++;
}

static void strip_comment(char *dst, const char *src){
    // copia hasta ';' (no incluido) y trimea
    size_t n = strlen(src);
    size_t i = 0;
    for(; i<n; i++){
        if(src[i]==';') break;
        dst[i] = src[i];
    }
    dst[i] = 0;
    rtrim_inplace(dst);
}

static int cmp_symbols(const void *a, const void *b){
    const Symbol *sa = (const Symbol*)a;
    const Symbol *sb = (const Symbol*)b;
    return strcmp(sa->name, sb->name);
}

int main(int argc, char **argv){
    if(argc != 3){
        fprintf(stderr, "Usage: %s input.asm output_base\n", argv[0]);
        return 1;
    }
    const char *infile = argv[1];
    const char *outbase = argv[2];

    char out_mem_path[512], out_bin_path[512], out_lst_path[512];
    snprintf(out_mem_path, sizeof(out_mem_path), "%s.mem", outbase);
    snprintf(out_bin_path, sizeof(out_bin_path), "%s.bin", outbase);
    snprintf(out_lst_path, sizeof(out_lst_path), "%s.lst", outbase);

    FILE *fin = fopen(infile,"r");
    if(!fin){ perror("fopen input"); return 1; }

    char raw_lines[MAX_LINES][MAX_LINE];
    int nlines = 0;
    while(fgets(raw_lines[nlines], MAX_LINE, fin)){
        nlines++;
        if(nlines>=MAX_LINES) die("too many lines");
    }
    fclose(fin);


    // =====================================
    // PASS 1: symbols and PC
    // =====================================
    int pc = 0;
    for(int i=0;i<nlines;i++){
        char line[MAX_LINE]; strcpy(line, raw_lines[i]);
        rtrim_inplace(line);
        char *s = ltrim(line);
        if(*s==0 || *s==';') continue;

        // label?
        char *colon = strchr(s, ':');
        if(colon){
            // label = token before ':'
            char label[64];
            int L = (int)(colon - s);
            if(L <= 0) die("empty label");
            strncpy(label, s, L);
            label[L]=0;
            // trim label
            char *lab = ltrim(label);
            // remove trailing spaces of label already done by rtrim_inplace
            add_symbol(lab, pc);
            s = ltrim(colon+1);
            if(*s==0) continue;
        }

        // tokenize remainder (without comments)
        char nocmt[MAX_LINE]; strip_comment(nocmt, s);
        if(nocmt[0]==0) continue;

        char *toks[MAX_TOKS]; int nt=0;
        tokenize(nocmt, toks, &nt);
        if(nt==0){ free_toks(toks, nt); continue; }

        if(toks[0][0]=='.'){
            if(strcmp(toks[0], ".org")==0){
                if(nt<2) die(".org expects a value");
                int ok; int v = parse_number(toks[1], &ok);
                if(!ok || v<0 || v>=MEM_SIZE) die(".org value invalid/out of range");
                pc = v;
            } else if(strcmp(toks[0], ".byte")==0){
                // count all bytes (support comma-separated in tokens)
                for(int k=1;k<nt;k++){
                    char *p = toks[k];
                    char *tok2 = strtok(p, ",");
                    while(tok2){
                        pc++;
                        tok2 = strtok(NULL, ",");
                    }
                }
            } else if(strcmp(toks[0], ".equ")==0){
                if(nt<3) die(".equ NAME VALUE");
                int ok; int v = parse_number(toks[2], &ok);
                if(!ok) die(".equ VALUE must be numeric for pass1");
                add_symbol(toks[1], v);
            } else {
                char msg[128]; snprintf(msg,sizeof(msg),"Unknown directive: %s", toks[0]); die(msg);
            }
            free_toks(toks, nt);
            continue;
        }

        // instruction size accounting
        char lower[64]; strtolower(lower, toks[0]);
        if(strcmp(lower,"halt")==0){
            pc += 1;
        } else if(strcmp(lower,"load")==0 || strcmp(lower,"add")==0 ||
                  strcmp(lower,"store")==0 || strcmp(lower,"jmp")==0 ||
                  strcmp(lower,"jz")==0){
            pc += 2;
        } else {
            char msg[128]; snprintf(msg,sizeof(msg),
                                     "Unknown mnemonic (pass1): %s (line %d)",
                                     toks[0], i+1);
            die(msg);
        }
        free_toks(toks, nt);
    }

    // =====================================
    // PASS 2: emit bytes and build listing
    // =====================================
    memset(out_mem, 0, sizeof out_mem);
    memset(used, 0, sizeof used);
    list_count = 0;
    pc = 0;

    for(int i=0;i<nlines;i++){
        char line[MAX_LINE]; strcpy(line, raw_lines[i]);
        rtrim_inplace(line);
        char *s = ltrim(line);
        if(*s==0 || *s==';'){
            continue;
        }

        // label?
        char *colon = strchr(s, ':');
        if(colon){
            s = ltrim(colon+1);
            if(*s==0) continue;
        }

        char source_clean[MAX_LINE]; strip_comment(source_clean, s);
        if(source_clean[0]==0) continue;

        char *toks[MAX_TOKS]; int nt=0;
        tokenize(source_clean, toks, &nt);
        if(nt==0){ free_toks(toks, nt); continue; }

        int start_addr = pc;
        int nbytes_emitted = 0;
        uint8_t b0=0, b1=0;

        if(toks[0][0]=='.'){
            if(strcmp(toks[0], ".org")==0){
                int ok; int v = parse_number(toks[1], &ok);
                if(!ok || v<0 || v>=MEM_SIZE) die(".org invalid in pass2");
                pc = v;
                // .org no emite bytes -> listing con 0 bytes
            } else if(strcmp(toks[0], ".byte")==0){
                for(int k=1;k<nt;k++){
                    char *p = toks[k];
                    char *tok2 = strtok(p, ",");
                    while(tok2){
                        int ok; int v = parse_number(tok2, &ok);
                        if(!ok){
                            int idx = find_symbol(tok2);
                            if(idx<0){
                                char msg[128];
                                snprintf(msg,sizeof(msg),
                                         "Unknown symbol in .byte: %s (line %d)",
                                         tok2, i+1);
                                die(msg);
                            }
                            v = symtab[idx].value;
                        }
                        if(pc<0 || pc>=MEM_SIZE) die(".byte out of mem range");
                        out_mem[pc] = (uint8_t)(v & 0xFF);
                        used[pc] = 1;
                        if(nbytes_emitted < 2){
                            if(nbytes_emitted==0) b0 = out_mem[pc];
                            else b1 = out_mem[pc];
                        }
                        nbytes_emitted++;
                        pc++;
                        tok2 = strtok(NULL, ",");
                    }
                }
            } else if(strcmp(toks[0], ".equ")==0){
                // no emite bytes; ya registrado en pass1
            } else {
                die("Unknown directive in pass2");
            }
            add_listing(start_addr, nbytes_emitted, b0, b1, source_clean);
            free_toks(toks, nt);
            continue;
        }

        char lower[64]; strtolower(lower, toks[0]);

        if(strcmp(lower,"halt")==0){
            if(pc<0 || pc>=MEM_SIZE) die("HALT out of range");
            out_mem[pc] = OP_HALT; used[pc]=1; b0 = out_mem[pc];
            nbytes_emitted=1; pc++;
        } else if(strcmp(lower,"load")==0 || strcmp(lower,"add")==0 ||
                  strcmp(lower,"store")==0 || strcmp(lower,"jmp")==0 ||
                  strcmp(lower,"jz")==0){
            if(nt<2) {
                char m[128];
                snprintf(m,sizeof(m),"Missing operand for %s (line %d)", toks[0], i+1);
                die(m);
            }
            int op;
            if     (strcmp(lower,"load")==0)  op = OP_LOAD;
            else if(strcmp(lower,"add")==0)   op = OP_ADD;
            else if(strcmp(lower,"store")==0) op = OP_STORE;
            else if(strcmp(lower,"jmp")==0)   op = OP_JMP;
            else                              op = OP_JZ;

            if(pc<0 || pc+1>=MEM_SIZE) die("instruction out of memory range");
            out_mem[pc] = (uint8_t)op; used[pc]=1; b0 = out_mem[pc]; pc++;
            int ok; int val = resolve_operand(toks[1], &ok);
            if(!ok){
                char m[128];
                snprintf(m,sizeof(m),"Undefined operand: %s (line %d)",
                         toks[1], i+1);
                die(m);
            }
            out_mem[pc] = (uint8_t)(val & 0xFF); used[pc]=1; b1 = out_mem[pc]; pc++;
            nbytes_emitted = 2;
        } else {
            char msg[128]; snprintf(msg,sizeof(msg),
                                     "Unknown mnemonic (pass2): %s (line %d)",
                                     toks[0], i+1);
            die(msg);
        }

        add_listing(start_addr, nbytes_emitted, b0, b1, source_clean);
        free_toks(toks, nt);
    }

    // last used address
    int last = 0;
    for(int i=0;i<MEM_SIZE;i++) if(used[i]) last = i;

    // write .mem
    FILE *fmem = fopen(out_mem_path,"w");
    if(!fmem){ perror("fopen .mem"); return 1; }
    for(int i=0;i<=last;i++) fprintf(fmem, "%02X\n", out_mem[i]);
    fclose(fmem);

    // write .bin
    FILE *fbin = fopen(out_bin_path,"wb");
    if(!fbin){ perror("fopen .bin"); return 1; }
    fwrite(out_mem, 1, last+1, fbin);
    fclose(fbin);

    // write .lst
    FILE *flst = fopen(out_lst_path,"w");
    if(!flst){ perror("fopen .lst"); return 1; }

    time_t now = time(NULL);
    char tbuf[128]; strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(flst, "; LISTING FILE\n; Source: %s\n; Generated: %s\n; Memory used: 0x%02X bytes (0..0x%02X)\n\n",
            infile, tbuf, last+1, last);

    fprintf(flst, "ADDR  BYTES      SOURCE\n");
    fprintf(flst, "====  =====     ========= \n");
    for(int i=0;i<list_count;i++){
        if(listing[i].nbytes==0){
            fprintf(flst, "      %-10s %s\n", "", listing[i].source);
        } else if(listing[i].nbytes==1){
            fprintf(flst, "%04X  %02X         %s\n", listing[i].addr,
                    listing[i].bytes[0], listing[i].source);
        } else {
            fprintf(flst, "%04X  %02X %02X     %s\n", listing[i].addr,
                    listing[i].bytes[0], listing[i].bytes[1], listing[i].source);
        }
    }

    // symbol table
    qsort(symtab, symcount, sizeof(Symbol), cmp_symbols);
    fprintf(flst, "\nSYMBOLS (%d):\n", symcount);
    for(int i=0;i<symcount;i++){
        fprintf(flst, "  %-20s = 0x%02X (%3d)\n",
                symtab[i].name, symtab[i].value, symtab[i].value);
        }
    fclose(flst);

    printf("Assembled %s -> %s.{mem,bin,lst} (last=0x%02X)\n",
           infile, outbase, last);
    return 0;
}
