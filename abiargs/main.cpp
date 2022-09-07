#include <cstdio>
#include <array>
#include <bitset>

// constexpr int abiArgs[] = { 1, 2, 8, 9 }; // Windows
constexpr int abiArgs[] = { 7, 6, 2, 1, 8, 9 }; // System V

void test(std::initializer_list<int> args) {
	enum class NodeType { Unused, Leaf, NonLeaf, Cycle };
	std::array<NodeType, 16> nodes{};
	std::array<int, 16> chainCount{};
	std::array<int, 16> argRegAssignments{};
	argRegAssignments.fill(-1);

	// Maps registers to their order in the function call ABI
	constexpr auto argRegOrder = [] {
		std::array<int, 16> argRegOrder{};
		argRegOrder.fill(17);
		int order = 0;
		for (auto& reg : abiArgs) {
			argRegOrder[reg] = order++;
		}
		return argRegOrder;
	}();

	auto evalArgRegAssignment = [&, argIndex = 0](int arg) mutable {
		argRegAssignments[abiArgs[argIndex]] = arg;
		if (nodes[abiArgs[argIndex]] != NodeType::NonLeaf) {
			nodes[abiArgs[argIndex]] = NodeType::Leaf;
		}
		nodes[arg] = NodeType::NonLeaf;
		++argIndex;
	};

	// Evaluate argument assignments and build the graph
	printf("args:");
	for (auto arg : args) {
		printf(" %d", arg);
		evalArgRegAssignment(arg);
	}
	printf("\n");

	// Find cycles and write down the necessary exchanges (in reverse order)
	std::array<std::pair<int, int>, 16> exchanges{};
	size_t exchangeCount = 0;
	for (int argReg = 0; argReg < 16; argReg++) {
		std::bitset<16> seenRegs;
		seenRegs.set(argReg);
		int assignedReg = argRegAssignments[argReg];
		while (assignedReg != -1 && nodes[assignedReg] != NodeType::Cycle) {
			if (seenRegs.test(assignedReg)) {
				int cycleStart = assignedReg;
				int reg = cycleStart;
				nodes[cycleStart] = NodeType::Cycle;
				do {
					if (nodes[argRegAssignments[reg]] != NodeType::Cycle) {
						exchanges[exchangeCount++] = std::make_pair(argRegAssignments[reg], reg);
					}
					reg = argRegAssignments[reg];
					nodes[reg] = NodeType::Cycle;
				} while (reg != cycleStart);
				break;
			}
			seenRegs.set(assignedReg);
			assignedReg = argRegAssignments[assignedReg];
		}
	}

	// Process assignment chains, starting from leaf nodes
	for (int reg = 0; reg < 16; reg++) {
		if (nodes[reg] == NodeType::Leaf) {
			int nextReg = argRegAssignments[reg];
			while (nodes[nextReg] != NodeType::Cycle) {
				++chainCount[nextReg];
				nextReg = argRegAssignments[nextReg];
			}
		}
	}

	// Emit instruction sequences; direct assignments first, then cycle exchanges
	for (int reg = 0; reg < 16; reg++) {
		if (nodes[reg] == NodeType::Leaf) {
			int currReg = reg;
			int nextReg = argRegAssignments[reg];
			while (nodes[nextReg] != NodeType::Cycle) {
				if (chainCount[nextReg] > 0) {
					--chainCount[nextReg];
				}
				if (chainCount[currReg] == 0 || chainCount[nextReg] == 0) {
					printf("  %d = %d\n", currReg, nextReg);
				}
				currReg = nextReg;
				nextReg = argRegAssignments[nextReg];
			}
			if (chainCount[currReg] == 0) {
				printf("  %d = %d\n", currReg, nextReg);
			}
		}
	}
	for (int i = exchangeCount - 1; i >= 0; i--) {
		auto& xchg = exchanges[i];
		printf("  %d <> %d\n", xchg.first, xchg.second);
	}
}

int main() {
	printf("abi order:");
	for (auto reg : abiArgs) {
		printf(" %d", reg);
	}
	printf("\n\n");

	/*for (int f = 0; f < 6; f++) {
		for (int e = 0; e < 6; e++) {
			for (int d = 0; d < 6; d++) {
				for (int c = 0; c < 6; c++) {
					for (int b = 0; b < 6; b++) {
						for (int a = 0; a < 6; a++) {
							test({ abiArgs[a], abiArgs[b], abiArgs[c], abiArgs[d], abiArgs[e], abiArgs[f] });
						}
					}
				}
			}
		}
	}
	return 0;*/

	auto args = { 6, 6, 7, 7, 7, 7 };

	enum class NodeType { Unused, Leaf, NonLeaf, Cycle };
	std::array<NodeType, 16> nodes{};
	std::array<int, 16> chainCount{};

	// ---------------------------------------------

	constexpr auto argRegOrder = [] {
		std::array<int, 16> argRegOrder{};
		argRegOrder.fill(17);
		int order = 0;
		for (auto& reg : abiArgs) {
			argRegOrder[reg] = order++;
		}
		return argRegOrder;
	}();

	std::bitset<16> handledArgs{};
	std::array<int, 16> argRegAssignments{};
	argRegAssignments.fill(-1);

	auto evalArgRegAssignment = [&, argIndex = 0](int arg) mutable {
		argRegAssignments[abiArgs[argIndex]] = arg;
		if (nodes[abiArgs[argIndex]] != NodeType::NonLeaf) {
			nodes[abiArgs[argIndex]] = NodeType::Leaf;
		}
		nodes[arg] = NodeType::NonLeaf;
		++argIndex;
	};

	printf("abi order:");
	for (auto reg : abiArgs) {
		printf(" %d", reg);
	}
	printf("\n\n");

	printf("args:");
	for (auto arg : args) {
		printf(" %d", arg);
		evalArgRegAssignment(arg);
	}
	printf("\n\n");

	printf("assignments:\n");
	for (int argReg = 0; argReg < 16; argReg++) {
		if (argRegAssignments[argReg] != -1) {
			printf("  %d = %d\n", argReg, argRegAssignments[argReg]);
		}
	}

	// Find cycles
	std::array<std::pair<int, int>, 16> exchanges{};
	size_t exchangeCount = 0;
	for (int argReg = 0; argReg < 16; argReg++) {
		std::bitset<16> seenRegs;
		seenRegs.set(argReg);
		int assignedReg = argRegAssignments[argReg];
		while (assignedReg != -1 && nodes[assignedReg] != NodeType::Cycle) {
			if (seenRegs.test(assignedReg)) {
				int cycleStart = assignedReg;
				int reg = cycleStart;
				nodes[cycleStart] = NodeType::Cycle;
				printf("found cycle: %d", cycleStart);
				do {
					if (nodes[argRegAssignments[reg]] != NodeType::Cycle) {
						exchanges[exchangeCount++] = std::make_pair(argRegAssignments[reg], reg);
					}
					reg = argRegAssignments[reg];
					nodes[reg] = NodeType::Cycle;
					printf(" <- %d", reg);
				} while (reg != cycleStart);
				printf("\n");
				break;
			}
			seenRegs.set(assignedReg);
			assignedReg = argRegAssignments[assignedReg];
		}
	}

	printf("chains:\n");
	for (int reg = 0; reg < 16; reg++) {
		if (nodes[reg] == NodeType::Leaf) {
			printf("  %d", reg);
			int nextReg = argRegAssignments[reg];
			while (nodes[nextReg] != NodeType::Cycle) {
				++chainCount[nextReg];
				printf(" <- %d", nextReg);
				nextReg = argRegAssignments[nextReg];
			}
			printf(" <- %d\n", nextReg);
		}
	}
	printf("\n");

	printf("cycle exchanges:\n");
	for (int i = exchangeCount - 1; i >= 0; i--) {
		auto& xchg = exchanges[i];
		printf("%d <> %d\n", xchg.first, xchg.second);
	}

	printf("\n------------------------------\n");

	printf("operation order:\n");
	for (int reg = 0; reg < 16; reg++) {
		if (nodes[reg] == NodeType::Leaf) {
			int currReg = reg;
			int nextReg = argRegAssignments[reg];
			while (nodes[nextReg] != NodeType::Cycle) {
				if (chainCount[nextReg] > 0) {
					--chainCount[nextReg];
				}
				if (chainCount[currReg] == 0 || chainCount[nextReg] == 0) {
					printf("  %d = %d\n", currReg, nextReg);
				}
				currReg = nextReg;
				nextReg = argRegAssignments[nextReg];
			}
			if (chainCount[currReg] == 0) {
				printf("  %d = %d\n", currReg, nextReg);
			}
		}
	}
	for (int i = exchangeCount - 1; i >= 0; i--) {
		auto& xchg = exchanges[i];
		printf("  %d <> %d\n", xchg.first, xchg.second);
	}

	return 0;
}
