# Contributing to Open Modbus OM-64DO

Thank you for considering a contribution to the **Open Modbus OM-64DO** project!  
We welcome improvements to firmware, hardware design, documentation, Modbus protocol handling, examples, and testing.

## How to Contribute

### 1. Fork the Repository

Click **Fork** on GitHub and clone your own fork:

```bash
git clone https://github.com/<your-user>/OM-64DO.git
cd OM-64DO
```

### 2. Create a Feature Branch

```bash
git checkout -b feature/my-improvement
```

Use meaningful branch names such as:

- fix/modbus-timeout
- feature/diagnostic-registers
- docs/add-wiring-section
- hw/rev1b-changes

### 3. Make Your Changes

When contributing to the project, please follow these guidelines to keep the codebase, hardware design, and documentation consistent and maintainable.

#### General Guidelines

- Keep each change **focused and atomic**  
- Write clear, descriptive commit messages  
- Avoid mixing unrelated fixes or features in the same commit  
- Update documentation whenever behavior or interfaces change  
- Test your changes thoroughly before submitting a pull request

#### Firmware Guidelines

If you are contributing firmware updates:

- Follow the existing coding style and file structure  
- Keep modules small and maintainable (GPIO, Modbus, config, drivers, etc.)  
- Validate all incoming Modbus data (addresses, frame lengths, baud codes)  
- Ensure Modbus RTU timing remains compliant  
- Test on **real hardware** when possible  
- Include Modbus frame traces or test instructions  

#### Hardware Guidelines

When contributing to schematic, PCB, or BOM:

- Respect industrial clearances, creepage, and current ratings  
- Document the electrical reasoning behind changes  
- Provide before/after screenshots for PCB layout changes  
- Update all affected files, including:
  - Schematic (`.kicad_sch`)
  - PCB (`.kicad_pcb`)
  - BOM (`bom.csv`)
  - Fabrication or assembly notes (if applicable)

Avoid increasing BOM cost unless the improvement is justified.

#### Documentation Guidelines

Documentation contributions are always welcome. Please:

- Use clear, concise language  
- Keep consistent formatting with existing sections  
- Include diagrams or examples when helpful  
- Update wiring examples, register maps, or Modbus instructions if changes affect them

#### Commit Message Format

Use clear, structured commit messages:

```text
<type>: <short description>

[optional longer description]
```

**Types:**

- fix: bug fix
- feat: new feature
- hw: hardware layout or schematic change
- docs: documentation changes
- refactor: code restructuring
- test: test-related updates

**Examples:**

```text
fix: correct coil index in Write Multiple Coils
hw: increase trace width on Bank C high-current paths
docs: add mixed-voltage wiring example
```

#### Submitting Your Changes

Once your changes are complete:

```bash
git add .
git commit -m "type: description of change"
git push origin <your-branch>
```

Then open a Pull Request against the main branch.
Maintainers may request changes before merging.

## Code of Conduct

By contributing, you agree to follow the project's [Code of Conduct](./CODE_OF_CONDUCT.md)

## Thank You

Your contributions help make the OM-64DO better, safer, and more capable.
We appreciate your support and collaboration!
