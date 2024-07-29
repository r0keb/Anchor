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
Now there is a preview of how it works.

1. First of all we need to choose the target process (in this case "notepad.exe") and load the Dll inside of it.
  ![Image1](Screenshots/DllInjection.jpg)

2. Once the Dll is loaded it is all done. We have to the hardware breakpoint is triggered so the custom code is executed.
![Image2](Screenshots/LoadedDll.jpg)

3. Triggering the Hardware Breakpoint...
![Image3](Screenshots/ExecutingTheHook.jpg)

4. Done! The code is executed and the Process still running normally.
![Image4](Screenshots/ExecutionCompleted.jpg)

### Credits
- [Blindside: A New Technique for EDR Evasion with Hardware Breakpoints](https://cymulate.com/blog/blindside-a-new-technique-for-edr-evasion-with-hardware-breakpoints)
- [StealthHook - A method for hooking a function without modifying memory protection](https://www.x86matthew.com/view_post?id=stealth_hook)
- [Function Hooking: Hardware Breakpoints](https://www.codereversing.com/archives/594)
- [hwbp4mw by @rad9800](https://github.com/rad9800/hwbp4mw/)

