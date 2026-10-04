#!/usr/bin/env python3
"""Minimal SH-3 user-mode interpreter for testing freestanding ELF programs.

Implements the SH-3 integer instruction set (no FPU, no DSP). A program
talks to the host with trapa:
  trapa #0x20  exit, status in r4
  trapa #0x21  write the byte in r4 to stdout
"""

import argparse
import struct
import sys

MEMORY_SIZE = 32 * 1024 * 1024
STACK_TOP = MEMORY_SIZE - 0x1000
MASK = 0xFFFFFFFF
RETURN_ADDRESS = 0xDEAD0000
IMMEDIATE = 0
DELAYED = 1


class EmulatorError(Exception):
    pass


def signed32(value):
    value &= MASK
    return value - 0x100000000 if value & 0x80000000 else value


def signed16(value):
    value &= 0xFFFF
    return value - 0x10000 if value & 0x8000 else value


def signed8(value):
    value &= 0xFF
    return value - 0x100 if value & 0x80 else value


def signed12(value):
    value &= 0xFFF
    return value - 0x1000 if value & 0x800 else value


class Machine:
    def __init__(self, trace=False):
        self.memory = bytearray(MEMORY_SIZE)
        self.registers = [0] * 16
        self.pc = 0
        self.pr = 0
        self.gbr = 0
        self.mach = 0
        self.macl = 0
        self.t = 0
        self.s = 0
        self.q = 0
        self.m = 0
        self.trace = trace
        self.exit_status = None
        self.output = bytearray()
        self.steps = 0

    def check(self, address, size):
        if address < 0x1000 or address + size > MEMORY_SIZE:
            raise EmulatorError("bad address 0x%08x at pc 0x%08x" % (address, self.pc))
        if address % size:
            raise EmulatorError("unaligned %d-byte access 0x%08x at pc 0x%08x" % (size, address, self.pc))

    def read8(self, address):
        address &= MASK
        self.check(address, 1)
        return self.memory[address]

    def read16(self, address):
        address &= MASK
        self.check(address, 2)
        return struct.unpack_from("<H", self.memory, address)[0]

    def read32(self, address):
        address &= MASK
        self.check(address, 4)
        return struct.unpack_from("<I", self.memory, address)[0]

    def write8(self, address, value):
        address &= MASK
        self.check(address, 1)
        self.memory[address] = value & 0xFF

    def write16(self, address, value):
        address &= MASK
        self.check(address, 2)
        struct.pack_into("<H", self.memory, address, value & 0xFFFF)

    def write32(self, address, value):
        address &= MASK
        self.check(address, 4)
        struct.pack_into("<I", self.memory, address, value & MASK)

    def load_elf(self, path):
        with open(path, "rb") as handle:
            data = handle.read()
        if data[:4] != b"\x7fELF" or data[4] != 1 or data[5] != 1:
            raise EmulatorError("not a 32-bit little-endian ELF")
        (machine,) = struct.unpack_from("<H", data, 18)
        if machine != 42:
            raise EmulatorError("not an SH ELF (machine %d)" % machine)
        entry, program_offset = struct.unpack_from("<II", data, 24)
        program_entry_size, program_count = struct.unpack_from("<HH", data, 42)
        for index in range(program_count):
            base = program_offset + index * program_entry_size
            kind, offset, virtual, _, file_size, memory_size = struct.unpack_from("<IIIIII", data, base)
            if kind != 1:
                continue
            if virtual + memory_size > STACK_TOP - 0x100000:
                raise EmulatorError("segment at 0x%08x does not fit" % virtual)
            self.memory[virtual:virtual + file_size] = data[offset:offset + file_size]
        self.pc = entry
        self.registers[15] = STACK_TOP
        self.pr = RETURN_ADDRESS

    def run(self, limit):
        while self.exit_status is None:
            if self.steps >= limit:
                raise EmulatorError("step limit reached at pc 0x%08x" % self.pc)
            self.step()
        return self.exit_status

    def step(self):
        address = self.pc
        if address == RETURN_ADDRESS:
            self.exit_status = self.registers[0] & 0xFF
            return
        result = self.execute(self.read16(address), address, False)
        if result is None:
            self.pc = (address + 2) & MASK
            return
        kind, target = result
        if kind == IMMEDIATE:
            self.pc = target
            return
        slot = self.read16(address + 2)
        if self.is_branch(slot):
            raise EmulatorError("branch in delay slot at 0x%08x" % (address + 2))
        self.execute(slot, address + 2, True)
        self.pc = target & MASK

    @staticmethod
    def is_branch(instruction):
        high = instruction >> 12
        if high in (0xA, 0xB):
            return True
        if high == 0x8 and ((instruction >> 8) & 0xF) in (0x9, 0xB, 0xD, 0xF):
            return True
        if high == 0x0 and (instruction & 0xFF) in (0x03, 0x23, 0x0B, 0x2B):
            return True
        if high == 0x4 and (instruction & 0xFF) in (0x0B, 0x2B):
            return True
        if high == 0xC and ((instruction >> 8) & 0xF) == 0x3:
            return True
        return False

    def sr(self):
        return (self.t) | (self.s << 1) | (self.q << 8) | (self.m << 9)

    def set_sr(self, value):
        self.t = value & 1
        self.s = (value >> 1) & 1
        self.q = (value >> 8) & 1
        self.m = (value >> 9) & 1

    def execute(self, instruction, address, in_slot):
        self.steps += 1
        r = self.registers
        n = (instruction >> 8) & 0xF
        m = (instruction >> 4) & 0xF
        low4 = instruction & 0xF
        low8 = instruction & 0xFF
        high = instruction >> 12
        if self.trace:
            sys.stderr.write("%08x: %04x  r0=%08x r1=%08x r4=%08x r5=%08x r15=%08x t=%d\n" % (
                address, instruction, r[0], r[1], r[4], r[5], r[15], self.t))

        if high == 0x0:
            if low4 == 0x4:
                self.write8(r[n] + r[0], r[m]); return None
            if low4 == 0x5:
                self.write16(r[n] + r[0], r[m]); return None
            if low4 == 0x6:
                self.write32(r[n] + r[0], r[m]); return None
            if low4 == 0x7:
                self.macl = (r[n] * r[m]) & MASK; return None
            if low4 == 0xC:
                r[n] = signed8(self.read8(r[m] + r[0])) & MASK; return None
            if low4 == 0xD:
                r[n] = signed16(self.read16(r[m] + r[0])) & MASK; return None
            if low4 == 0xE:
                r[n] = self.read32(r[m] + r[0]); return None
            if low4 == 0xF:
                left = signed32(self.read32(r[n])); r[n] = (r[n] + 4) & MASK
                right = signed32(self.read32(r[m])); r[m] = (r[m] + 4) & MASK
                total = ((self.mach << 32) | self.macl) + left * right
                total &= 0xFFFFFFFFFFFFFFFF
                self.mach = total >> 32; self.macl = total & MASK
                return None
            if low8 == 0x02:
                r[n] = self.sr(); return None
            if low8 == 0x12:
                r[n] = self.gbr; return None
            if low8 == 0x03:
                self.pr = (address + 4) & MASK; return (DELAYED, (address + 4 + r[n]) & MASK)
            if low8 == 0x23:
                return (DELAYED, (address + 4 + r[n]) & MASK)
            if instruction == 0x0008:
                self.t = 0; return None
            if instruction == 0x0018:
                self.t = 1; return None
            if instruction == 0x0028:
                self.mach = 0; self.macl = 0; return None
            if instruction == 0x0048:
                self.s = 0; return None
            if instruction == 0x0058:
                self.s = 1; return None
            if instruction == 0x0009:
                return None
            if instruction == 0x0019:
                self.m = 0; self.q = 0; self.t = 0; return None
            if low8 == 0x29:
                r[n] = self.t; return None
            if instruction == 0x000B:
                return (DELAYED, self.pr)
            if low8 == 0x0A:
                r[n] = self.mach; return None
            if low8 == 0x1A:
                r[n] = self.macl; return None
            if low8 == 0x2A:
                r[n] = self.pr; return None
        elif high == 0x1:
            self.write32(r[n] + low4 * 4, r[m]); return None
        elif high == 0x2:
            if low4 == 0x0:
                self.write8(r[n], r[m]); return None
            if low4 == 0x1:
                self.write16(r[n], r[m]); return None
            if low4 == 0x2:
                self.write32(r[n], r[m]); return None
            if low4 == 0x4:
                value = r[m]; r[n] = (r[n] - 1) & MASK; self.write8(r[n], value); return None
            if low4 == 0x5:
                value = r[m]; r[n] = (r[n] - 2) & MASK; self.write16(r[n], value); return None
            if low4 == 0x6:
                value = r[m]; r[n] = (r[n] - 4) & MASK; self.write32(r[n], value); return None
            if low4 == 0x7:
                self.q = r[n] >> 31; self.m = r[m] >> 31; self.t = self.q ^ self.m; return None
            if low4 == 0x8:
                self.t = 1 if (r[n] & r[m]) == 0 else 0; return None
            if low4 == 0x9:
                r[n] &= r[m]; return None
            if low4 == 0xA:
                r[n] ^= r[m]; return None
            if low4 == 0xB:
                r[n] |= r[m]; return None
            if low4 == 0xC:
                value = r[n] ^ r[m]
                self.t = 1 if any(((value >> shift) & 0xFF) == 0 for shift in (0, 8, 16, 24)) else 0
                return None
            if low4 == 0xD:
                r[n] = ((r[m] << 16) | (r[n] >> 16)) & MASK; return None
            if low4 == 0xE:
                self.macl = ((r[n] & 0xFFFF) * (r[m] & 0xFFFF)) & MASK; return None
            if low4 == 0xF:
                self.macl = (signed16(r[n]) * signed16(r[m])) & MASK; return None
        elif high == 0x3:
            if low4 == 0x0:
                self.t = 1 if r[n] == r[m] else 0; return None
            if low4 == 0x2:
                self.t = 1 if r[n] >= r[m] else 0; return None
            if low4 == 0x3:
                self.t = 1 if signed32(r[n]) >= signed32(r[m]) else 0; return None
            if low4 == 0x4:
                self.div1(n, m); return None
            if low4 == 0x5:
                total = r[n] * r[m]
                self.mach = (total >> 32) & MASK; self.macl = total & MASK; return None
            if low4 == 0x6:
                self.t = 1 if r[n] > r[m] else 0; return None
            if low4 == 0x7:
                self.t = 1 if signed32(r[n]) > signed32(r[m]) else 0; return None
            if low4 == 0x8:
                r[n] = (r[n] - r[m]) & MASK; return None
            if low4 == 0xA:
                total = r[n] - r[m] - self.t
                self.t = 1 if total < 0 else 0
                r[n] = total & MASK; return None
            if low4 == 0xB:
                total = signed32(r[n]) - signed32(r[m])
                self.t = 1 if total < -0x80000000 or total > 0x7FFFFFFF else 0
                r[n] = total & MASK; return None
            if low4 == 0xC:
                r[n] = (r[n] + r[m]) & MASK; return None
            if low4 == 0xD:
                total = signed32(r[n]) * signed32(r[m])
                total &= 0xFFFFFFFFFFFFFFFF
                self.mach = total >> 32; self.macl = total & MASK; return None
            if low4 == 0xE:
                total = r[n] + r[m] + self.t
                self.t = 1 if total > MASK else 0
                r[n] = total & MASK; return None
            if low4 == 0xF:
                total = signed32(r[n]) + signed32(r[m])
                self.t = 1 if total < -0x80000000 or total > 0x7FFFFFFF else 0
                r[n] = total & MASK; return None
        elif high == 0x4:
            if low4 == 0xC:
                amount = r[m]
                if not amount & 0x80000000:
                    r[n] = (r[n] << (amount & 0x1F)) & MASK
                elif amount & 0x1F == 0:
                    r[n] = MASK if r[n] & 0x80000000 else 0
                else:
                    r[n] = (signed32(r[n]) >> (((~amount) & 0x1F) + 1)) & MASK
                return None
            if low4 == 0xD:
                amount = r[m]
                if not amount & 0x80000000:
                    r[n] = (r[n] << (amount & 0x1F)) & MASK
                elif amount & 0x1F == 0:
                    r[n] = 0
                else:
                    r[n] = r[n] >> (((~amount) & 0x1F) + 1)
                return None
            if low4 == 0xF:
                raise EmulatorError("mac.w not implemented")
            if low8 == 0x00 or low8 == 0x20:
                self.t = r[n] >> 31; r[n] = (r[n] << 1) & MASK; return None
            if low8 == 0x01:
                self.t = r[n] & 1; r[n] >>= 1; return None
            if low8 == 0x21:
                self.t = r[n] & 1; r[n] = (signed32(r[n]) >> 1) & MASK; return None
            if low8 == 0x04:
                self.t = r[n] >> 31; r[n] = ((r[n] << 1) | self.t) & MASK; return None
            if low8 == 0x05:
                self.t = r[n] & 1; r[n] = (r[n] >> 1) | (self.t << 31); return None
            if low8 == 0x24:
                carry = r[n] >> 31; r[n] = ((r[n] << 1) | self.t) & MASK; self.t = carry; return None
            if low8 == 0x25:
                carry = r[n] & 1; r[n] = (r[n] >> 1) | (self.t << 31); self.t = carry; return None
            if low8 == 0x08:
                r[n] = (r[n] << 2) & MASK; return None
            if low8 == 0x18:
                r[n] = (r[n] << 8) & MASK; return None
            if low8 == 0x28:
                r[n] = (r[n] << 16) & MASK; return None
            if low8 == 0x09:
                r[n] >>= 2; return None
            if low8 == 0x19:
                r[n] >>= 8; return None
            if low8 == 0x29:
                r[n] >>= 16; return None
            if low8 == 0x10:
                r[n] = (r[n] - 1) & MASK; self.t = 1 if r[n] == 0 else 0; return None
            if low8 == 0x11:
                self.t = 1 if signed32(r[n]) >= 0 else 0; return None
            if low8 == 0x15:
                self.t = 1 if signed32(r[n]) > 0 else 0; return None
            if low8 in (0x02, 0x12, 0x22, 0x03, 0x13):
                value = {0x02: self.mach, 0x12: self.macl, 0x22: self.pr, 0x03: self.sr(), 0x13: self.gbr}[low8]
                r[n] = (r[n] - 4) & MASK; self.write32(r[n], value); return None
            if low8 in (0x06, 0x16, 0x26, 0x07, 0x17):
                value = self.read32(r[n]); r[n] = (r[n] + 4) & MASK
                if low8 == 0x06: self.mach = value
                elif low8 == 0x16: self.macl = value
                elif low8 == 0x26: self.pr = value
                elif low8 == 0x07: self.set_sr(value)
                else: self.gbr = value
                return None
            if low8 == 0x0A:
                self.mach = r[n]; return None
            if low8 == 0x1A:
                self.macl = r[n]; return None
            if low8 == 0x2A:
                self.pr = r[n]; return None
            if low8 == 0x0E:
                self.set_sr(r[n]); return None
            if low8 == 0x1E:
                self.gbr = r[n]; return None
            if low8 == 0x0B:
                target = r[n]; self.pr = (address + 4) & MASK; return (DELAYED, target)
            if low8 == 0x2B:
                return (DELAYED, r[n])
            if low8 == 0x1B:
                value = self.read8(r[n]); self.t = 1 if value == 0 else 0
                self.write8(r[n], value | 0x80); return None
        elif high == 0x5:
            r[n] = self.read32(r[m] + low4 * 4); return None
        elif high == 0x6:
            if low4 == 0x0:
                r[n] = signed8(self.read8(r[m])) & MASK; return None
            if low4 == 0x1:
                r[n] = signed16(self.read16(r[m])) & MASK; return None
            if low4 == 0x2:
                r[n] = self.read32(r[m]); return None
            if low4 == 0x3:
                r[n] = r[m]; return None
            if low4 == 0x4:
                value = signed8(self.read8(r[m])) & MASK
                if n != m: r[m] = (r[m] + 1) & MASK
                r[n] = value; return None
            if low4 == 0x5:
                value = signed16(self.read16(r[m])) & MASK
                if n != m: r[m] = (r[m] + 2) & MASK
                r[n] = value; return None
            if low4 == 0x6:
                value = self.read32(r[m])
                if n != m: r[m] = (r[m] + 4) & MASK
                r[n] = value; return None
            if low4 == 0x7:
                r[n] = (~r[m]) & MASK; return None
            if low4 == 0x8:
                value = r[m]
                r[n] = (value & 0xFFFF0000) | ((value & 0xFF) << 8) | ((value >> 8) & 0xFF); return None
            if low4 == 0x9:
                r[n] = ((r[m] >> 16) | (r[m] << 16)) & MASK; return None
            if low4 == 0xA:
                total = 0 - r[m] - self.t
                self.t = 1 if total < 0 else 0
                r[n] = total & MASK; return None
            if low4 == 0xB:
                r[n] = (-r[m]) & MASK; return None
            if low4 == 0xC:
                r[n] = r[m] & 0xFF; return None
            if low4 == 0xD:
                r[n] = r[m] & 0xFFFF; return None
            if low4 == 0xE:
                r[n] = signed8(r[m]) & MASK; return None
            if low4 == 0xF:
                r[n] = signed16(r[m]) & MASK; return None
        elif high == 0x7:
            r[n] = (r[n] + signed8(low8)) & MASK; return None
        elif high == 0x8:
            if n == 0x0:
                self.write8(r[m] + low4, r[0]); return None
            if n == 0x1:
                self.write16(r[m] + low4 * 2, r[0]); return None
            if n == 0x4:
                r[0] = signed8(self.read8(r[m] + low4)) & MASK; return None
            if n == 0x5:
                r[0] = signed16(self.read16(r[m] + low4 * 2)) & MASK; return None
            if n == 0x8:
                self.t = 1 if r[0] == (signed8(low8) & MASK) else 0; return None
            if n in (0x9, 0xB, 0xD, 0xF):
                if in_slot:
                    raise EmulatorError("branch in delay slot at 0x%08x" % address)
                taken = self.t == (1 if n in (0x9, 0xD) else 0)
                if not taken:
                    return None
                destination = (address + 4 + signed8(low8) * 2) & MASK
                if n in (0x9, 0xB):
                    return (IMMEDIATE, destination)
                return (DELAYED, destination)
        elif high == 0x9:
            r[n] = signed16(self.read16(address + 4 + low8 * 2)) & MASK; return None
        elif high == 0xA:
            return (DELAYED, (address + 4 + signed12(instruction) * 2) & MASK)
        elif high == 0xB:
            self.pr = (address + 4) & MASK
            return (DELAYED, (address + 4 + signed12(instruction) * 2) & MASK)
        elif high == 0xC:
            if n == 0x0:
                self.write8(self.gbr + low8, r[0]); return None
            if n == 0x1:
                self.write16(self.gbr + low8 * 2, r[0]); return None
            if n == 0x2:
                self.write32(self.gbr + low8 * 4, r[0]); return None
            if n == 0x3:
                self.trap(low8); return None
            if n == 0x4:
                r[0] = signed8(self.read8(self.gbr + low8)) & MASK; return None
            if n == 0x5:
                r[0] = signed16(self.read16(self.gbr + low8 * 2)) & MASK; return None
            if n == 0x6:
                r[0] = self.read32(self.gbr + low8 * 4); return None
            if n == 0x7:
                r[0] = ((address & ~3) + 4 + low8 * 4) & MASK; return None
            if n == 0x8:
                self.t = 1 if (r[0] & low8) == 0 else 0; return None
            if n == 0x9:
                r[0] &= low8; return None
            if n == 0xA:
                r[0] ^= low8; return None
            if n == 0xB:
                r[0] |= low8; return None
        elif high == 0xD:
            r[n] = self.read32((address & ~3) + 4 + low8 * 4); return None
        elif high == 0xE:
            r[n] = signed8(low8) & MASK; return None
        raise EmulatorError("unimplemented instruction %04x at 0x%08x" % (instruction, address))

    def div1(self, n, m):
        raise EmulatorError("div1 not implemented")

    def trap(self, number):
        if number == 0x20:
            self.exit_status = self.registers[4] & 0xFF
        elif number == 0x21:
            self.output.append(self.registers[4] & 0xFF)
        else:
            raise EmulatorError("unknown trap %d" % number)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf")
    parser.add_argument("--trace", action="store_true")
    parser.add_argument("--limit", type=int, default=200000000)
    arguments = parser.parse_args()
    machine = Machine(trace=arguments.trace)
    machine.load_elf(arguments.elf)
    try:
        status = machine.run(arguments.limit)
    except EmulatorError as error:
        sys.stdout.buffer.write(machine.output)
        sys.stdout.flush()
        sys.stderr.write("shemu: %s\n" % error)
        return 125
    sys.stdout.buffer.write(machine.output)
    sys.stdout.flush()
    return status


if __name__ == "__main__":
    sys.exit(main())
