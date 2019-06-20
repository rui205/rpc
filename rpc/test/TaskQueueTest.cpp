#include <iostream>
#include "../TaskQueue.h"

using namespace std;

/*task queue test*/

int main() {
    rpc::TaskQueue* queue = new rpc::TaskQueue();
    if (queue == NULL) {
        cout << "new task queue failed" << endl;
        return -1;
    }

    queue->init();

    cout << "test begin:" << endl;

    int count = 15000;
    while (count --) {
        rpc::task_item_t* item = (rpc::task_item_t*)calloc(1, sizeof(rpc::task_item_t));
        if (item == NULL) {
            cout << "item nil" << endl;
            return -1;
        }
        if (item != NULL) {
            queue->push(item);
      //      cout << "count: " << count << endl;
        }
    }

    while (!queue->empty()) {
        rpc::task_item_t* tmp = queue->pop();
        if (tmp == NULL) {
            cout << "tmp is nil" << endl;
            return -1;
        }

        queue->free(tmp);
    }

    cout << "realloc count: " << queue->getReallocCount() << endl;

    cout << "test end" << endl;

    return 0;
}