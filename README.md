IIn the console::init() there is function with lambda that is rerouting system prints:
       esp_log_set_vprintf([](const char *format, va_list args)
                                {
                                      // calculate needed size
                                      va_list args_copy;
                                      va_copy(args_copy, args);
                                      int len = vsnprintf(nullptr, 0, format, args_copy);
                                      va_end(args_copy);

                                      if (len < 0)
                                            return 0;

                                      // Allocate exact buffer (+1 for null terminator)
                                      char *buffer = (char *)malloc(len + 1);
                                      if (!buffer)
                                            return 0;

                                      // Format into buffer
                                      vsnprintf(buffer, len + 1, format, args);

                                      if (printToSerial)
                                            Serial.print(buffer);
                                      if (printToServer)
                                            WebSerial.print(buffer);

                                      free(buffer);

                                      return len; // it must return what og was returning. It's not important how many bytes were printed but how much oryginally should be
                                });
During run time i call digitalWrite with wrong values to trigger print. It partially works,
It misses part of prints still. Implement a way that will reroute all of them.  This is my read from UART0:

E (20366) gpio: gpio_set_level(227): GPIO output gpio_num error

[console.cpp::execute_imp:36] new cmd: 'digitalwrite(-1, -1)'

[105069][E][esp32-hal-gpio.c:102] __pinMode(): Invalid pin selected

[console.cpp::extractBool:117] extractBool err outOfScope: -1 

E (105242) gpio: gpio_set_level(227): GPIO output gpio_num error


and this is from webSerial:
[console.cpp::execute_imp:36] new cmd: 'digitalwrite(-1, -1)'     

[console.cpp::extractBool:117] extractBool err outOfScope: -1

E (105242) gpio: gpio_set_level(227): GPIO output gpio_num error

