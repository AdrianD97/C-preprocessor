#define VAR0 1

#define VAR1 VAR0 + 8

int main() {
    int y = VAR0 + 1 + VAR1;
    printf("%d\n", VAR0);
#undef VAR0
    printf("%d\n", VAR0);

    y = VAR1;
    return 0;
}