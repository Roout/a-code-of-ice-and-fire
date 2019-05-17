#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <limits>
#include <functional>
#include <map>
#include <set>
#include <cassert>
#include <algorithm>
using namespace std;
/*
	Constraints
Allotted response time to output is <= 50ms.
Allotted response time to output on the first turn is <= 1000ms.
*/
struct Vec2 {
	int x, y;
};
istream& operator >>(istream&in, Vec2& v) {
	in >> v.x >> v.y;
	return in;
}
ostream& operator <<(ostream&out, Vec2& v) {
	out << "{" << v.x << " " << v.y << "}";
	return out;
}

struct Player {

	void Read() {
		cin >> m_gold; cin.ignore();
		cin >> m_income; cin.ignore();
	}
	// data
	int m_gold;
	int m_income;
};
enum class Tile : char {
	blocked = '#',
	neutral = '.',
	mActive = 'O',
	mInactive = 'o',
	eActive = 'X',
	eInactive = 'x'
};
constexpr char toChar(Tile ty) noexcept {
	return static_cast<char>(ty);
}
constexpr Tile toTile(char c) noexcept {
	return static_cast<Tile>(c);
}
struct Map {
	
	void Read() {
		for (int i = 0; i < SIZE; i++) {
			string s; cin >> s;  cin.ignore();
			for (int j = 0; j < SIZE; j++) 
				m_map[i][j] = toTile(s[j]);
		}
	}
	// data
	static const int SIZE{ 12 };
	array<array<Tile, SIZE>, SIZE> m_map;
};
enum class BType: int {
	HQ = 1
};
constexpr int toInt(BType ty) noexcept {
	return static_cast<int>(ty);
}
constexpr BType toBType(int ty) noexcept {
	return static_cast<BType>(ty);
}

struct Building {
	int		m_owner;
	BType	m_type;
	Vec2	m_pos;

	bool IsMine() const noexcept {
		return m_owner == 0;
	}
};
struct BuildingManager {

	void Read() {
		int buildingCount;
		cin >> buildingCount; cin.ignore();

		m_buildings.clear();
		m_buildings.reserve(buildingCount);

		for (int i = 0; i < buildingCount; i++) {
			int owner;
			int buildingType;
			Vec2 pos;
			cin >> owner >> buildingType >> pos; cin.ignore();
			m_buildings.emplace_back(owner, ::toBType(buildingType), pos);
		}
	}
	// data
	vector<Building> m_buildings;
};

struct Unit {
	void Read() {
		cin >> m_owner >> m_id >> m_level >> m_pos; 
		cin.ignore();
	}

	bool IsMine() const noexcept {
		return m_owner == 0;
	}

	int	m_owner;
	int m_id;
	int m_level;
	Vec2 m_pos;
};

struct UnitManager {
	void Read() {
		int unitCount;
		cin >> unitCount; cin.ignore();

		m_units.resize(unitCount);
		for (auto& unit: m_units) {
			unit.Read();
		}
	}

	vector<Unit> m_units;
};
namespace commands 
{
	void Move(int id, Vec2 pos) {
		cout << "MOVE " << id << " " << pos.x << " " << pos.y << ";";
	}
	void Train(int level, Vec2 pos) {
		cout << "TRAIN " << level << " " << pos.x << " " << pos.y << ";";
	}
	void Wait() {
		cout << "WAIT;";
	}
	void Msg(const char* msg) {
		cout << "MSG " << msg << ";";
	}
}

struct Game {

	Game() {
	
	}
	
	void Loop() {
		while (true) {
			this->Read();
			

			cout << "WAIT" << endl;
		}
	}
	
	void Read() {
		m_me.Read();
		m_enemy.Read();
		m_map.Read();
		m_bManager.Read();
		m_uManger.Read();
	}

private:
	Map		m_map;
	Player	m_me, 
			m_enemy;
	BuildingManager m_bManager;
	UnitManager		m_uManger;
};
int main() {
	int numberMineSpots;
	cin >> numberMineSpots; cin.ignore();
	for (int i = 0; i < numberMineSpots; i++) {
		int x, y;
		cin >> x >> y; cin.ignore();
	}
	Game().Loop();
}