# bdb: Bobby's Debugger

## Installation

TODO

## Developer Setup

TODO

## Building

```bash
mkdir build
cd build
cmake ..
cmake --build .
./tools/bdb
```

## Testing

```bash
mkdir build
cd build
cmake ..
cmake --build .
./test/tests
```

## License

Apache-2.0

## Improvements

-[ ] Rename `Process` class to `Tracee`.
-[ ] Rename `_should_terminate_session_on_end` to `_should_terminate_on_detach`.
-[ ] Construct a `ProcessStoppedEvent` directly from a `waitpid` system call.
-[ ] Support tracing of multiple tracees.
