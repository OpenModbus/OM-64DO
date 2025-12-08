# Security Policy

## Reporting a Vulnerability

We take the security and reliability of the **Open Modbus OM-64DO** hardware, firmware, and software ecosystem seriously.  
If you discover a security issue, we strongly encourage you to report it responsibly and privately.

### 📧 How to Report
Please send all vulnerability reports to:

[**contact@sszczep.dev**](mailto:contact@sszczep.dev)

Alternatively, you may open a **private security advisory** through the project’s GitHub repository:

[**https://github.com/OpenModbus/OM-64DO/security/advisories**](https://github.com/OpenModbus/OM-64DO/security/advisories)

### 🔒 Responsible Disclosure
Please **do not** publicly disclose the vulnerability before we have confirmed and addressed it.  
Early disclosure may put existing users at risk.

### 🛠 Security Issue Handling
Once a vulnerability is confirmed:

1. We assign a **severity rating** (Low / Medium / High / Critical)  
2. We begin **patch development** for firmware, hardware notes, or documentation  
3. We prepare a **coordinated disclosure timeline** with you  
4. A fix is published along with a **security advisory**  
5. Credits are provided to reporters (optional and only with consent)

### 🧪 What to Include in Your Report
To help us investigate efficiently, please include:

- Description of the vulnerability  
- Steps to reproduce  
- Affected firmware version / hardware revision  
- Expected behavior vs actual behavior  
- Impact assessment (if known)  
- Any proof-of-concept code, test scripts, or Modbus sequences  

### ⚙ Scope of Security Reports
We accept vulnerability reports related to:

- Firmware or bootloader security  
- Modbus RTU protocol handling  
- Configuration register integrity  
- Buffer overflows or malformed frame handling  
- Hardware-level protection issues  
- Repository code or documentation security concerns  

Issues **not** considered security vulnerabilities:

- Incorrect Modbus configuration by users  
- Miswiring or electrical overload conditions  
- Out-of-spec voltage, current, or installation  
- Third-party library bugs (unless they affect OM-64DO security)

### 🙏 Thank You
We appreciate the work of security researchers and users who help improve the safety of the project.  
Responsible disclosure ensures the **OM-64DO** remains reliable in industrial and automation environments.
