struct S {
    int value;
    const char get() const { return value; } // не изменяет объект
    void set(int v) { value = v; }    // может изменять объект
};


int main(){
    
}