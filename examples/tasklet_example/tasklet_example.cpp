#include <aetherium/concurrent/channel.hpp>
#include <aetherium/concurrent/tasklet.hpp>

#include <iostream>

int main() {
  using namespace Aetherium::Concurrent;

  std::cout << "-- C++ Tasklet and Channel Imitation --" << std::endl;

  TaskletManager manager(4);

  std::cout << "\n--- Example 1: Basic Tasklet ---" << std::endl;

  go(&manager, [] { std::cout << "Hello from goroutine 1!" << std::endl; });

  go<std::function<void()>>(
      &manager, [] { std::cout << "Hello from goroutine 2!" << std::endl; });

  std::cout << "\n--- Example 2: Tasklet with Panic ---" << std::endl;
  go(&manager, [] {
    std::cout << "Tasklet about to panic..." << std::endl;

    tasklet_panic("Something went terribly wrong!");

    std::cout << "This line will not be reached." << std::endl;
  });

  std::cout << "\n--- Example 3: Unbuffered Channel with Close ---"
            << std::endl;

  Channel<int> unbuffered_ch_closed;
  go(&manager, [&] {
    std::cout << "Sender (unbuffered, closed): Sending 10" << std::endl;
    unbuffered_ch_closed.send(10);

    std::cout << "Sender (unbuffered, closed): Sent 10" << std::endl;
    std::cout << "Sender (unbuffered, closed): Sending 20" << std::endl;
    unbuffered_ch_closed.send(20);

    std::cout << "Sender (unbuffered, closed): Sent 20" << std::endl;
    std::cout << "Sender (unbuffered, closed): Closing channel" << std::endl;
    unbuffered_ch_closed.close();

    try {
      std::cout << "Sender (unbuffered, closed): "
                   "Attempting to send 30 after close (will panic)"
                << std::endl;
      unbuffered_ch_closed.send(30);
    } catch (const std::runtime_error &e) {
      std::cout << "Sender (unbuffered, closed): "
                   "Caught expected error: "
                << e.what() << std::endl;
    }
  });

  go(&manager, [&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto result1 = unbuffered_ch_closed.receive();

    if (result1.second)
      std::cout << "Receiver (unbuffered, closed): Received " << result1.first
                << std::endl;
    else
      std::cout << "Receiver (unbuffered, closed): "
                   "Channel closed and empty."
                << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto result2 = unbuffered_ch_closed.receive();

    if (result2.second)
      std::cout << "Receiver (unbuffered, closed): Received " << result2.first
                << std::endl;
    else
      std::cout << "Receiver (unbuffered, closed): "
                   "Channel closed and empty."
                << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto result3 = unbuffered_ch_closed.receive();

    if (result3.second)
      std::cout << "Receiver (unbuffered, closed): Received " << result3.first
                << std::endl;
    else
      std::cout << "Receiver (unbuffered, closed): "
                   "Channel closed and empty."
                << std::endl;
  });

  std::cout << "\n--- Example 4: Buffered Channel with Close ---" << std::endl;

  Channel<std::string> buffered_ch_closed(2);
  go(&manager, [&] {
    std::cout << "Sender (buffered, closed): Sending 'apple'" << std::endl;
    buffered_ch_closed.send("apple");

    std::cout << "Sender (buffered, closed): Sending 'banana'" << std::endl;
    buffered_ch_closed.send("banana");

    std::cout << "Sender (buffered, closed): Closing channel" << std::endl;
    buffered_ch_closed.close();

    try {
      std::cout << "Sender (buffered, closed): "
                   "Attempting to send 'cherry' after close (will panic)"
                << std::endl;
      buffered_ch_closed.send("cherry");
    } catch (const std::runtime_error &e) {
      std::cout << "Sender (buffered, closed): Caught expected error: "
                << e.what() << std::endl;
    }
  });

  go(&manager, [&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string fruit1;
    bool ok1 = buffered_ch_closed.receive().second;

    if (ok1)
      std::cout << "Receiver (buffered, closed): "
                   "Received fruit 1 (channel still open or had data)"
                << std::endl;
    else
      std::cout << "Receiver (buffered, closed): "
                   "Channel closed and empty (fruit 1)."
                << std::endl;

    std::string fruit2;
    bool ok2 = buffered_ch_closed.receive().second;

    if (ok2)
      std::cout << "Receiver (buffered, closed): "
                   "Received fruit 2 (channel still open or had data)"
                << std::endl;
    else
      std::cout << "Receiver (buffered, closed): "
                   "Channel closed and empty (fruit 2)."
                << std::endl;

    std::string fruit3;
    bool ok3 = buffered_ch_closed.receive().second;
    if (ok3)
      std::cout << "Receiver (buffered, closed): "
                   "Received fruit 3 (channel still open or had data)"
                << std::endl;
    else
      std::cout << "Receiver (buffered, closed): "
                   "Channel closed and empty (fruit 3)."
                << std::endl;
  });

  std::cout << "\n--- Example 5: Try-Send/Try-Receive with Close ---"
            << std::endl;

  Channel<double> try_ch_closed(1);
  go(&manager, [&] {
    std::cout << "Try-Sender (closed): Attempting to send 1.1" << std::endl;

    if (try_ch_closed.try_send(1.1))
      std::cout << "Try-Sender (closed): Sent 1.1 successfully." << std::endl;
    else
      std::cout << "Try-Sender (closed): "
                   "Failed to send 1.1 (channel full/no receiver/closed)."
                << std::endl;
    try_ch_closed.close();

    std::cout << "Try-Sender (closed): Channel closed." << std::endl;
    std::cout
        << "Try-Sender (closed): Attempting to send 2.2 after close (will fail)"
        << std::endl;

    if (try_ch_closed.try_send(2.2))
      std::cout << "Try-Sender (closed): Sent 2.2 successfully." << std::endl;
    else
      std::cout << "Try-Sender (closed): "
                   "Failed to send 2.2 (channel full/no receiver/closed)."
                << std::endl;
  });

  go(&manager, [&] {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    double val;
    std::cout << "Try-Receiver (closed): Attempting to receive" << std::endl;

    if (try_ch_closed.try_receive(val))
      std::cout << "Try-Receiver (closed): Received " << val << " successfully."
                << std::endl;
    else
      std::cout << "Try-Receiver (closed): "
                   "Failed to receive (channel empty/closed)."
                << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "Try-Receiver (closed): "
                 "Attempting to receive again after close (will fail)"
              << std::endl;

    if (try_ch_closed.try_receive(val))
      std::cout << "Try-Receiver (closed): Received " << val << " successfully."
                << std::endl;
    else
      std::cout << "Try-Receiver (closed): "
                   "Failed to receive (channel empty/closed)."
                << std::endl;
  });

  manager.wait_for_completion();
  std::cout << "\n--- Main function finished ---" << std::endl;

  return 0;
}
