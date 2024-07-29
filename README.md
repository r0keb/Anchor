# Anchor

### About
Injectable DLL that sets hardware breakpoints on NT functions.

### Functionality
- Once is injected, it sets hardware breakpoint on NT functions.
- After the call of any of the hooked functions, the execution flow goes to the "malicious code" which is a MessageBox for the POC.
- Once our code is executed. The hardware breakpoints are cleaned and everything goes normally. (i chose to execute the custom code once for the POC, but it can be modified)

### Disclaimer
- There is no opsec (Dll and DllInjector) in the code, no CRT library removal, no indirect syscalls, no API hashing... This is because I only wanted to explore more in depth the hardware breakpoint hook on a remote process.
- There is a Dll injector included:
  - The code is not commented because it is not the main objective of this repo.
  - It is a very precarious injector but again, it is not the main objective.

### Preview
Now there is a preview of how it works. (target process: "notepad.exe")

![Image1](Screenshots/DllInjection.jpg)
