#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <functional>
#include <queue>
#include <cassert>
#include <optional>
#include <algorithm>
using namespace std;
/*
	Constraints
Allotted response time to output is <= 50ms.
Allotted response time to output on the first turn is <= 1000ms.
*/

struct Vec2 {
	int x, y;

	constexpr Vec2(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}

	int Distanse(const Vec2& rsh) const noexcept {
		return abs(x - rsh.x) + abs(y - rsh.y);
	}
	constexpr bool operator < (const Vec2& rsh) const noexcept {
		if (x == rsh.x) return y < rsh.y;
		return x < rsh.x;
	}
	constexpr bool operator == (const Vec2& rsh) const noexcept {
		return x == rsh.x && y == rsh.y;
	}
	friend constexpr Vec2 operator + (const Vec2& lsh, const Vec2& rsh) noexcept {
		return { lsh.x + rsh.x, lsh.y + rsh.y };
	}
	
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
	int CanCreate(int level) const noexcept {
		if (level == 1) {
			return m_gold / 10;
		}
		else if (level == 2) {
			return m_gold / 20;
		}
		else {
			return m_gold / 30;
		}
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
	Tile Get(Vec2 pos) const noexcept {
		return m_map[pos.y][pos.x];
	}
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
constexpr bool IsValid(Vec2 pos) noexcept {
	return	pos.x >= 0 && pos.y >= 0 &&
			pos.x < Map::SIZE && pos.y < Map::SIZE;
}

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

	Building() {};
	Building(int owner, BType ty, Vec2 pos) :
		m_owner(owner), m_type(ty), m_pos(pos) {}

	int		m_owner;
	BType	m_type;
	Vec2	m_pos;

	bool IsMy() const noexcept {
		return m_owner == 0;
	}
	bool IsHQ() const noexcept {
		return m_type == BType::HQ;
	}
};
struct Mine {
	Vec2 m_pos;
};
struct BuildingManager {
	void ReadMines() {
		int numberMineSpots;
		cin >> numberMineSpots; cin.ignore();

		m_mines.resize(numberMineSpots);

		for (int i = 0; i < numberMineSpots; i++) {
			Vec2 pos; cin >> pos; cin.ignore();
			m_mines[i].m_pos = pos;
		}
	}
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
	vector<Mine> m_mines;
};

struct Unit {
	Unit() {};

	Unit(int owner, int id, int level, Vec2 pos) :
		m_owner(owner),
		m_id(id),
		m_level(level),
		m_pos(pos) {};

	void Read() {
		cin >> m_owner >> m_id >> m_level >> m_pos; 
		cin.ignore();
	}

	bool IsMy() const noexcept {
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

	optional<Unit> GetUnitAt(const Vec2&pos) const noexcept {
		auto it = find_if(m_units.begin(), m_units.end(), [&pos](const Unit&u) {
			return u.m_pos == pos;
		});
		return (it == m_units.end() ? nullopt : make_optional(*it));
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

struct Data {
	Map		m_map;
	Player	m_me,
			m_enemy;
	BuildingManager m_bManager;
	UnitManager		m_uManger;

	void Update() {
		m_me.Read();
		m_enemy.Read();
		m_map.Read();
		m_bManager.Read();
		m_uManger.Read();
	}
	void Init() {
		m_bManager.ReadMines();
	}
};
class Commander {
public:	
	Commander(Data *data) : 
		m_data(data) 
	{
		auto& buildings{ m_data->m_bManager.m_buildings };
		auto hq = find_if(buildings.begin(), buildings.end(), [](auto&b) {
			return b.IsMy() && b.IsHQ();
		});
		assert(hq != buildings.end());
		m_mHQ = hq->m_pos;
		//--
		hq = find_if(buildings.begin(), buildings.end(), [](auto&b) {
			return (!b.IsMy() && b.IsHQ());
		});
		assert(hq != buildings.end());
		m_eHQ = hq->m_pos;
	}

	/* TRAINING:
	Find all my boarder tiles 
	Foreach my tile check boarder tiles and give score:
		+ tile is enemy HQ [max score]
		+ tile is with opponent's unit (weaker or equel in level) [unit's cost + default enemy tile score]
		- tile is enemy Bridge [number of opponent tiles + units cost if there are any in connected component]
		- tile is next to my Bridge [can reinforce bridge? YES = |CC|  + units cost there, NO = default tile score + 1] / 2
		- tile is next to my inactive [|CC| + units cost there] // build bridge 
		+ tile is opponents active [3] // default enemy tile score 
		+ tile is opponents inactive [2]
		- else [1]
	Sort by score (if is equel, use distance to enemy HQ)
	While can train => train
	*/

	// bfs
	vector<pair<Vec2,int>> GetBoarderTiles() const noexcept {
		array<Vec2, 4> shift { Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		auto& map{ m_data->m_map };
		
		bool visited[Map::SIZE][Map::SIZE] = {};
		visited[m_mHQ.y][m_mHQ.x] = true;

		vector<pair<Vec2, int>> boarderTiles;
		boarderTiles.reserve(10); //to avoid allocations

		queue<Vec2> Q;
		Q.push(m_mHQ);

		while (!Q.empty()) {
			auto top{ Q.front() };
			Q.pop();

			for (auto& sh : shift) {
				auto tile{ top + sh };
				auto type{ map.Get(tile) };
				if (IsValid(tile) && 
					!visited[tile.y][tile.x] && 
					type != Tile::blocked ) 
				{
					visited[tile.y][tile.x] = true;

					if (type == Tile::mActive) {
						Q.push(tile);
					}
					else {
						boarderTiles.emplace_back(tile,0);
					}
				}
			}
		}

		return boarderTiles;
	}

	void AssignScore() const noexcept {
		auto boarderTiles{ this->GetBoarderTiles() };
		for (auto & [pos,score] : boarderTiles) 
		{
			score = 1; // default
			
		}
	}
	
	int EvalScore(Vec2& pos) const noexcept {

		auto& map{ m_data->m_map };
		auto& bManager{ m_data->m_bManager };
		auto& uManager{ m_data->m_uManger };

		const int mx = 1000;
		const int mn = 1;
		
		const int activeTileScore = 3;
		const int inactiveTileScore = 2;
		const int defaultScore = mn;

		auto score{ defaultScore };

		auto optUnit{ m_data->m_uManger.GetUnitAt(pos) };

		if (m_eHQ == pos) {
			score = mx;
		} 
		else if (optUnit.has_value()) {
			// tile is with opponent's unit (weaker or equel in level)
			// [unit's cost + default enemy tile score]
			auto& unit{ optUnit.value() };
			if (unit.m_level == 1) {
				score = 10 + activeTileScore;
			}
		}
		else if (map.Get(pos) == Tile::eActive) {
			score = activeTileScore;
		}
		else if (map.Get(pos) == Tile::eInactive) {
			score = inactiveTileScore;
		}
		// bridge
	}

private:
	Data* m_data;
	// deduced:
	Vec2 m_mHQ, m_eHQ;

};

class Bridges {
public:
	Bridges(Map* map) :
		m_map(map),
		m_shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} }
	{//default ctor
	}

	vector<Vec2>&& GetBridges(Tile type) noexcept {
		this->Clear();

		for (int y = 0; y < Map::SIZE; y++) {
			for (int x = 0; x < Map::SIZE; x++) {
				Vec2 v{ x, y };
				if (!m_visited[y][x] && m_map->Get(v) == type)
					this->Dfs(v, Vec2{ -1,-1 }, type);
			}
		}

		return move(m_bridges);
	}
private:
	void Dfs(Vec2 v, Vec2 p, Tile type) noexcept {
		m_visited[v.y][v.x] = true;

		m_start[v.y][v.x] = m_finish[v.y][v.x] = m_timer++;

		for (auto& sh : m_shift) {
			auto to{ v + sh };
			auto ty{ m_map->Get(to) };
			if (type != ty) continue;
			if (to == p)  continue;
			if (m_visited[to.y][to.x])
				m_finish[v.y][v.x] = min(m_finish[v.y][v.x], m_start[to.y][to.x]);
			else {
				Dfs(to, v, type);
				m_finish[v.y][v.x] = min(m_finish[v.y][v.x], m_finish[to.y][to.x]);
				if (m_finish[to.y][to.x] > m_start[v.y][v.x])
				{//found bridge
					cerr << "Bridge: " << v << " - " << to << endl;
				}
			}
		}
	}
	void Clear() noexcept {
		for (int i = 0; i < Map::SIZE; i++) {
			for (int j = 0; j < Map::SIZE; j++) {
				m_visited[i][j] = false;
				m_start[i][j] = m_finish[i][j] = 0;
			}
		}
		m_bridges.clear();
		m_timer = 0;
	}
private:
	vector<Vec2> m_bridges;

	array<Vec2, 4> m_shift;
	bool	m_visited[Map::SIZE][Map::SIZE];
	int		m_start[Map::SIZE][Map::SIZE];
	int		m_finish[Map::SIZE][Map::SIZE];
	int		m_timer;

	Map* m_map;
};

struct Game {
	void Loop() {
		m_data.Init();
		while (true) {
			m_data.Update();
			
			Bridges bridges(&m_data.m_map);
			bridges.GetBridges(Tile::mActive);

			cout << "TRAIN 1 1 0; MOVE 1 11 11;" << endl;
		}
	}
	
	

private:
	Data m_data;
};

int main() {
	Game().Loop();
}