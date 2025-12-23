#include <iostream>
using namespace std;
int a[100] = {};
int n = 0;

void quickSort(int low, int high)
{
    if (low >= high) return;

    int i = low;
    int j = high;
    int mind = (low + high) / 2;
    int p = a[mind];

    while (i <= j)
    {
        while (a[i] < p) i++;
        while (a[j] > p) j--;
        if (i < j) {
            swap(a[i], a[j]);
            i++;
            j--;
        }
    }
    if (low < j) quickSort(low, j);
    if (high > i) quickSort(i, high);
    
}

int main() 
{
    cout << "input element number: ";
    cin >> n;
    cout << "\ninput elements: " << endl;
    for (int i = 0; i < n; i++) {
        cin >> a[i];
    }

    quickSort(0, n-1);

    for (int i = 0; i< n; i++) {
        cout << a[i] << " ";
    }
    cout << endl;

    return 0;
}
