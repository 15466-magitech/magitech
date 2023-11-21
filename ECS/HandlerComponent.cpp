//
// Created by Russell Emerine on 11/18/23.
//

#include "HandlerComponent.hpp"

struct TestHandler : HandlerComponent<TestHandler, int, int, int> {
    using HandlerComponent<TestHandler, int, int, int>::HandlerComponent;
    
    static int test_handle_all(int a, int b) {
        TestHandler tester([](int x, int y) {
            return x + y;
        });
        
        int total = 0;
        system([&](TestHandler &handler) {
            int result = handler.handle(a, b);
            std::cout << total << std::endl;
            total += result;
        });
        return total;
    }
};
