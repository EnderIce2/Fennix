# Fennix Style Guide

This document outlines the coding standards and style guidelines for contributing to the Fennix project. Consistent code helps improve readability, maintainability, and collaboration.

## General Naming Conventions

- **CamelCase** is used for all identifiers:
  - Function and global declaration names must start with an uppercase letter.
    - Example: `InitializeKernel`, `LoadModule`
  - Local variable names must start with a lowercase letter.
    - Example: `bufferSize`, `inputString`

## Code Formatting

- **File formatting** should follow the "Visual Studio" style:
  - Use `Tab` for indentation. Each `Tab` should represent one level of indentation.
  - Consistently format files by using "Format Document" (available in editors like Visual Studio or VS Code).
  - Code blocks must align correctly with their containing structure (e.g., functions, loops).

### Example

```c
void InitializeKernel()
{
	int bufferSize = 1024;
	char *inputString = AllocateMemory(bufferSize);
	if (inputString == NULL)
	{
		HandleError("Memory allocation failed");
		return;
	}

	int status = LoadModules(inputString);
	if (status == 0)
		return 0;

	// Continue with the initialization
}
```

## Commenting Guidelines

- Write clear and concise comments to describe the purpose and functionality of the code if necessary. (**Avoid unnecessary comments.**)
  - A good explanation of this is [here](https://youtu.be/Bf7vDBBOBUA).
- Use inline comments sparingly to clarify complex logic.
- Document public APIs using Doxygen-style comments.

### Example

```c
/**
 * @brief Initializes the kernel.
 * 
 * This function initializes the kernel by loading modules and setting up the environment.
 */
void InitializeKernel();


void ProcessData()
{
	/* Read data from the buffer */
	size_t bytesRead = ReadData(buffer, MAX_BUFFER_SIZE);
	if (bytesRead == 0) /* No data read */
	{
		HandleError("No data read");
		return;
	}
}
```

## Guidelines for File Structure

- Group related functions together.
- Use a consistent order for declarations: global variables, private functions, public functions.
- Separate sections of code with blank lines for better readability.
- Avoid having too many functions in a single file.
- End each file with a newline character.

## Best Practices

- Ensure code is clean and free of unnecessary or redundant elements.
- Follow the DRY principle (Don’t Repeat Yourself) by reusing existing functions and utilities.

## Automated Tools

- Use linters or formatters integrated into your editor to ensure consistent style.
- Run tests and validate changes before submitting.

---

Adhering to these guidelines ensures that the Fennix codebase remains robust, readable, and easy to maintain. Thank you for your effort and commitment to quality!
