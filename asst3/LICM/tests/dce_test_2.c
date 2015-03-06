
int foo(int a, int b){
    a = a - 3;
    b = a * b;
    a = a / (a + b);
    return a * b;
}


int main(){
    int r1, r2, r3;
    int x, y, z;

    r1 = 1;
    r2 = 2;
    r3 = 3;
    z = r1 + r3/r2;

    if(r2 > foo(1, 5)){
        r1 = 5;
        z = z * 2;
        r3 = r1 * z + r3;
    }
    else{
        r1 = 6;
        z = r1 / r3;
        r3 = z * 5 + r2 * z ;
    }

    r1 = r2 + 1;
    r2 = r2 + 2;
    //r3 = r2 + r3;
    r1 = r2 + 3;

    y = 20;
    z = y * z;

    return r3;
}
