#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <limits>
#include <array>
#include <tuple>
#include <set>
#include <functional>
#include <sstream>
#include <queue>
#include <cassert>
#include <optional>
#include <random>
#include <algorithm>
#include <type_traits> //is_invocable_v
using namespace std;
/*
	Constraints
Allotted response time to output is <= 50ms.
Allotted response time to output on the first turn is <= 1000ms.
*/

struct Vec2 {
	int x, y;

	constexpr Vec2() :x(0), y(0) {};
	constexpr Vec2(int x_, int y_) : x(x_), y(y_) {}

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
	constexpr bool operator != (const Vec2& rsh) const noexcept {
		return !(*this == rsh);
	}
	friend constexpr Vec2 operator + (const Vec2& lsh, const Vec2& rsh) noexcept {
		return { lsh.x + rsh.x, lsh.y + rsh.y };
	}
	
};

istream& operator >>(istream&in, Vec2& v) {
	in >> v.x >> v.y;
	return in;
}
ostream& operator <<(ostream&out, const  Vec2& v) {
	out << "{" << v.x << " " << v.y << "}";
	return out;
}

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

namespace ScoreDistribution
{
	const int mxScore{ 1000 };
	const int mnScore{ 1 };

	const int activeTileScore{ 3 };
	const int inactiveTileScore{ 2 };
	const int defaultScore{ mnScore };

	const int costByLevel[3] = { 10, 20, 30 };
	const int salaryByLevel[3] = { 1, 4, 20 };

	const int incomeFromMine{ 4 };
	const int towerCost{ 15 };
	const int minMineCost{ 20 };
};
namespace sd = ScoreDistribution;

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
	HQ = 0,
	Mine = 1,
	Tower = 2
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
	bool IsMine() const noexcept {
		return m_type == BType::Mine;
	}
	bool IsTower() const noexcept {
		return m_type == BType::Tower;
	}
};

struct Mine {
	Vec2 m_pos;

	Mine() {};
	Mine(Vec2 p) :m_pos(p) {};

	bool operator == (const Mine& m) const noexcept {
		return m.m_pos == m_pos;
	}
};

struct Player {

	void Read() {
		cin >> m_gold; cin.ignore();
		cin >> m_income; cin.ignore();
	}

	bool CanCreateUnits(const vector<int> & levels ) const noexcept {
		Player dummy = *this; //copy
		bool is = true;
		for (auto level : levels) {
			if (!dummy.CanCreateUnit(level, 1)) {
				is = false; break;
			}
			dummy.CreateUnit(level, 1);
		}
		return is;
	}
	
	bool CanCreateUnit(int level, int incomeFromUnitPos) const noexcept {
		bool is = true;
		// if we create this unit, next time we need to pay @salary (upkeep)
	//	int upkeep{ m_upkeep + sd::salaryByLevel[level - 1] };
		int gold{ m_gold - sd::costByLevel[level-1] };

		if (gold < 0) {
			is = false;
		}
		//update income
		int income{ m_income + incomeFromUnitPos  - sd::salaryByLevel[level - 1] };

		if (gold + income < 0) {
			is = false;
		}

		return is;
	}
	bool CanCreateBuilding(int cost) const noexcept {
		return  m_gold - cost >= 0;
	}
	void CreateBuilding(int cost, int incomeFromCreation) noexcept {
		m_gold -= cost;
		m_income += incomeFromCreation; // 0 for towers
	}
	void CreateUnit(int level, int incomeFromUnitPos) noexcept {
		m_gold -= sd::costByLevel[level - 1];
		m_income += incomeFromUnitPos - sd::salaryByLevel[level - 1];
		m_upkeep += sd::salaryByLevel[level - 1];
	}
// data
	int m_gold;
	int m_income;
	int m_upkeep;
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

	void AddBuilding(int owner, BType type, Vec2 pos) {
		m_buildings.emplace_back(owner, type, pos);
	}

	optional<Building> GetBuildingAt(Vec2 pos) const noexcept {
		auto it = find_if(m_buildings.begin(), m_buildings.end(), [&pos](const Building& b) {
			return b.m_pos == pos;
		});
		return (it == m_buildings.end() ? nullopt : make_optional(*it));
	}

	bool IsProtected(Vec2 p, const Map& map) const noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };

		Tile ty{ map.Get(p) };

		if (find_if(m_buildings.begin(), m_buildings.end(), [&p, &shift, &map, &ty](auto& b) {
			if (b.IsTower() && ty == map.Get(b.m_pos)) {
				if (b.m_pos == p) return true;

				for (auto& sh : shift) {
					auto towerNeighbor{ sh + b.m_pos };
					if (towerNeighbor == p)
					{ // tower and neighbor has the same owner
						return true;
					}
				}
			}
			return false;
		}) != m_buildings.end())
		{
			return true;
		}
		return false;
	}

	vector<Building> GetMines(bool isMy) const noexcept {
		vector<Building> mines;
		for_each(m_buildings.begin(), m_buildings.end(),[&mines,&isMy](auto& b) {
			if(isMy == b.IsMy() && b.IsMine()) mines.emplace_back(b);
		});
		return mines;
	}

	vector<Building> GetTowers(bool isMy) const noexcept {
		vector<Building> towers;
		for_each(m_buildings.begin(), m_buildings.end(), [&towers, &isMy](auto& b) {
			if (isMy == b.IsMy()&& b.IsTower()) towers.emplace_back(b);
		});
		return towers;
	}

	size_t Count(BType ty, bool isMy) const noexcept {
		return (size_t)count_if(m_buildings.begin(), m_buildings.end(), [&ty, &isMy](auto& b) {
			return (isMy == b.IsMy() && b.m_type == ty);
		});
	}

	bool IsMineSpot(Vec2 p) const noexcept {
		return any_of(m_mines.begin(), m_mines.end(), [&p](auto&mine) {return mine.m_pos == p; });
	}

	// data
	vector<Building> m_buildings;// owned buildings
	vector<Mine> m_mines; // all mines
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

	bool CanKill(const Unit& u) const noexcept {
		return u.m_level < m_level || m_level == 3;
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

		m_units.clear();
		m_units.resize(unitCount);
		for (auto& unit: m_units) {
			unit.Read();
		}
	}

	void AddUnit(int owner, int id, int level, Vec2 pos) {
		m_units.emplace_back(owner, id, level, pos);
	}

	optional<Unit> GetUnitAt(const Vec2&pos) const noexcept {
		auto it = find_if(m_units.begin(), m_units.end(), [&pos](const Unit&u) {
			return u.m_pos == pos;
		});
		return (it == m_units.end() ? nullopt : make_optional(*it));
	}

	void MarkUnitForRemove(Vec2 pos) noexcept {
		auto it = find_if(m_units.begin(), m_units.end(), [&pos](const Unit&u) {
			return u.m_pos == pos;
		});
		if (it != m_units.end()) it->m_id = m_idForRemove;
	}
// TODO: CLEAN UP!
	void RemoveMarkedUnits() noexcept{
		int id = m_idForRemove;
		m_units.erase(
			remove_if(m_units.begin(), m_units.end(), [id](const Unit&u) {
				if (u.m_id == id) cerr << "Remove unit at " << u.m_pos << endl;
				return u.m_id == id;
			}),
			m_units.end()
		);
	}
	vector<Unit> m_units;
	int m_idForRemove{ -1000 };
};
namespace commands 
{
	string Move(int id, Vec2 pos) {
		stringstream ss;
		ss << "MOVE " << id << " " << pos.x << " " << pos.y << ";";
		return ss.str();
	}
	string Train(int level, Vec2 pos) {
		stringstream ss;
		ss << "TRAIN " << level << " " << pos.x << " " << pos.y << ";";
		return ss.str();
	}
	string Wait() {
		stringstream ss;
		ss << "WAIT;";
		return ss.str();
	}
	string Msg(const char* msg) {
		stringstream ss;
		ss << "MSG " << msg << ";";
		return ss.str();
	}
	string Build(BType ty, Vec2 pos) {
		stringstream ss;
		ss << "BUILD " << (ty == BType::Mine? "MINE": "TOWER") << " " << pos.x << " " << pos.y << ";";
		return ss.str();
	}
}

struct Data {
	Map		m_map;
	Player	m_me,
			m_enemy;
	BuildingManager m_bManager;
	UnitManager		m_uManager;

	void Update() {
		m_me.Read();
		m_enemy.Read();
		m_map.Read();
		m_bManager.Read();
		m_uManager.Read();

		m_me.m_upkeep = 0;
		for (auto& unit : m_uManager.m_units) {
			if(unit.IsMy()) m_me.m_upkeep += sd::salaryByLevel[unit.m_level - 1];
		}
	}
	void Init() {
		m_bManager.ReadMines();
	}
};

/* look for bridges */
class Bridges {
public:
	Bridges(Map* map) :
		m_map(map),
		m_shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} }
	{//default ctor
		m_mHQ = Vec2{ 0,0 }, m_eHQ = Vec2{ 11,11 };
		if (map->Get(Vec2{ 0,0 }) == Tile::eActive)
			swap(m_mHQ, m_eHQ);
	}

	vector<pair<Vec2, Vec2>>&& GetBridges(Tile type) noexcept {
		this->Clear();

		Vec2 start{ type == Tile::eActive ? m_eHQ : m_mHQ};
		assert(m_bridges.empty());
		this->Dfs(start, Vec2{ -1,-1 }, type);
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
			if (!IsValid(to)) continue;
			auto ty{ m_map->Get(to) };
			if (type != ty || to == p) continue;
			if (m_visited[to.y][to.x])
				m_finish[v.y][v.x] = min(m_finish[v.y][v.x], m_start[to.y][to.x]);
			else {
				Dfs(to, v, type);
				m_finish[v.y][v.x] = min(m_finish[v.y][v.x], m_finish[to.y][to.x]);
				if (m_finish[to.y][to.x] > m_start[v.y][v.x])
				{//found bridge
				//	 cerr << "Bridge: " << v << " - " << to << endl;
					m_bridges.emplace_back(v, to);
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
	vector<pair<Vec2, Vec2>> m_bridges;

	array<Vec2, 4> m_shift;
	bool	m_visited[Map::SIZE][Map::SIZE];
	int		m_start[Map::SIZE][Map::SIZE];
	int		m_finish[Map::SIZE][Map::SIZE];
	int		m_timer;

	Map* m_map;
	Vec2 m_mHQ, m_eHQ;
};

/* find components connected by bridge in the graph */
class CCSearch {
public:
	CCSearch(Map*map, UnitManager* uManager, BuildingManager * bManager) :
		m_map(map),
		m_uManager(uManager),
		m_bManager(bManager),
		m_shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} }
	{//default ctor
	}

	int GetScoreAfterBridge(Vec2 start, Vec2 bridge, Tile type) noexcept {
		int score{ 0 };
		auto&map{ m_map };
		// CC before bridge
		auto pred = [&bridge,&type, &map](Vec2 v) { return  ( !(v == bridge) && type == map->Get(v) ); };
		auto calc = [](Vec2 v) { return 1; };
		// remove overhead from score caculation
		this->Dfs<decltype(pred),decltype(calc)>(start, score, pred, calc);
		// CC after bridge with calc score eval
		score = 0;
		this->Dfs(bridge, Vec2{-1,-1}, score, type);
		return score;
	}
	void ChangeTilesAfterTheBridge(Vec2 start, Vec2 bridge, Tile from, Tile to) noexcept {
		int score{ 0 };
		auto&map{ m_map };
		auto&uManager{ m_uManager };
		// CC before bridge to mark as visited and skip it later
		auto pred = [&bridge, &from, &map](Vec2 v) { return  (!(v == bridge) && from == map->Get(v)); };
		auto calc = [](Vec2 v) { return 1; }; // do nothing
		// remove overhead from score caculation
		this->Dfs<decltype(pred), decltype(calc)>(start, score, pred, calc);
		auto modify = [&bridge, &from, &to, &map, &uManager](Vec2 v) {
			bool is{ from == map->Get(v) };
			if (is) {
				map->m_map[v.y][v.x] = to;
				//// remove units from manager!
				//uManager->MarkUnitForRemove(v);
				//cerr << "Marked: " << v << endl;
			}
			return is;
		};
		this->Dfs<decltype(modify), decltype(calc)>(bridge, score, modify, calc);
	//	uManager->RemoveMarkedUnits();
	}
	/* used to calculate CC */
	template <class Pred, class Calc>
	void Dfs(Vec2 v, int& counter, Pred pred, Calc calcFn) {
		static_assert(is_invocable_v<Pred, Vec2>, "can't invoce predicate");
		static_assert(is_invocable_v<Calc, Vec2>, "can't invoce calc func");
		m_visited[v.y][v.x] = true;
		counter+= calcFn(v);
		for (auto& sh : m_shift) {
			auto to{ v + sh };
			if (!IsValid(to) || m_visited[to.y][to.x]) continue;
			if (pred(to))
				Dfs(to, counter, pred, calcFn);
		}
	}

	void Dfs(Vec2 v, int& visits, Tile type) {
		m_visited[v.y][v.x] = true;
		visits++;
		for (auto& sh : m_shift) {
			auto to{ v + sh };
			if (!IsValid(to) || m_visited[to.y][to.x]) continue;
			auto ty{ m_map->Get(to) };
			if (type == ty)
				Dfs(to, visits, type);
		}
	}
// TODO: remove [score calculation as template parameter in dfs above].
	void Dfs(Vec2 v, Vec2 bridge, int& score, Tile type) {
		m_visited[v.y][v.x] = true;

		auto optUnit{ m_uManager->GetUnitAt(v) };
		auto optBuilding{ m_bManager->GetBuildingAt(v) };
// calc score:
		auto tileScore{ 0 };
		if (optBuilding.has_value()) {
			tileScore += optBuilding.value().IsTower() ? sd::towerCost : sd::minMineCost;
		}
		else if (optUnit.has_value()) {
			// tile is with opponent's unit (weaker or equel in level)
			// [unit's cost + default enemy tile score]
			auto& unit{ optUnit.value() };
			tileScore += sd::costByLevel[unit.m_level - 1];
		}

		if (type == Tile::eActive || type == Tile::mActive) {
			tileScore += sd::activeTileScore;
		}
		else if (type == Tile::eInactive || type == Tile::mInactive) {
			tileScore += sd::inactiveTileScore;
		}
		
		score += tileScore;
//---
		for (auto& sh : m_shift) {
			auto to{ v + sh };
			if (!IsValid(to) || to == bridge || m_visited[to.y][to.x]) continue;
			auto ty{ m_map->Get(to) };
			if (type == ty)
				Dfs(to, bridge, score, type);
		}
	}
	void Clear() noexcept {
		for (int i = 0; i < Map::SIZE; i++) {
			for (int j = 0; j < Map::SIZE; j++) {
				m_visited[i][j] = false;
			}
		}
	}

	// bfs: self-clearing
	vector<pair<Vec2, int>> GetBoarderTiles(Tile type) {

		this->Clear();

		auto	mHQ = Vec2{ 0,0 }, 
				eHQ = Vec2{ 11,11 };
		if (m_map->Get(Vec2{ 0,0 }) == Tile::eActive)
			swap(mHQ, eHQ);

		Vec2 HQ{ type == Tile::mActive? mHQ : eHQ};
	
		m_visited[HQ.y][HQ.x] = true;

		vector<pair<Vec2, int>> boarderTiles;
		boarderTiles.reserve(10); //to avoid allocations

		queue<Vec2> Q;
		Q.push(HQ);

		while (!Q.empty()) {
			auto top{ Q.front() };
			Q.pop();

			for (auto& sh : m_shift) {
				auto tile{ top + sh };
				if (IsValid(tile) &&
					!m_visited[tile.y][tile.x] &&
					m_map->Get(tile) != Tile::blocked)
				{
					m_visited[tile.y][tile.x] = true;

					if (m_map->Get(tile) == type) {
						Q.push(tile);
					}
					else {
						boarderTiles.emplace_back(tile, 0);
					}
				}
			}
		}

		return boarderTiles;
	}

	vector<pair<Vec2, int>> GetOutline(Tile type) {
		bool m_added[Map::SIZE][Map::SIZE];
		for (int i = 0; i < Map::SIZE; i++) {
			for (int j = 0; j < Map::SIZE; j++) {
				m_visited[i][j] = m_added[i][j] = false;
			}
		}

		auto	mHQ = Vec2{ 0,0 },
				eHQ = Vec2{ 11,11 };
		if (m_map->Get(Vec2{ 0,0 }) == Tile::eActive)
			swap(mHQ, eHQ);

		Vec2 HQ{ type == Tile::mActive ? mHQ : eHQ };

		m_visited[HQ.y][HQ.x] = true;

		vector<pair<Vec2, int>> boarderTiles;
		boarderTiles.reserve(10); //to avoid allocations

		queue<Vec2> Q;
		Q.push(HQ);

		while (!Q.empty()) {
			auto top{ Q.front() };
			Q.pop();

			for (auto& sh : m_shift) {
				auto tile{ top + sh };
				if (IsValid(tile) &&
					!m_visited[tile.y][tile.x] &&
					m_map->Get(tile) != Tile::blocked)
				{
					if (m_map->Get(tile) == type) {
						m_visited[tile.y][tile.x] = true;
						Q.push(tile);
					}
					else if(!m_added[top.y][top.x]) {
						boarderTiles.emplace_back(top, 0);
						m_added[top.y][top.x] = true;
					}
				}
			}
		}

		return boarderTiles;
	}
	// Dijkstra: has error when we're removing towers! 
	// protected tiles still calculated!
	// ****
	// Look for cheapest path to enemy HQ and it's cost
	void FindPath(Vec2 start, Vec2 finish) {
		this->ClearDijkstra();
		 // calculate cost to conquire this tile
		auto Cost = [this](Vec2 pos) {
			int cost{ 0 };
			bool isProtected{ m_bManager->IsProtected(pos,*m_map) };
			auto unit{ m_uManager->GetUnitAt(pos) };

			if (isProtected) {
				cost = sd::costByLevel[2];
			} 
			else if (unit.has_value()) {
				cost = sd::costByLevel[min(unit->m_level + 1, 3) - 1];
			}
			else {
				cost = sd::costByLevel[0];
			}
			return cost;
		};

		DijkstraPriority Q;
		Q.emplace(start, start, 0);
		m_cost[start.y][start.x] = 0;
		while (!Q.empty()) {
			auto top = Q.top();
			Q.pop();

			if (m_visited[top.position.y][top.position.x]) continue;

			m_visited[top.position.y][top.position.x] = true;
			m_parent[top.position.y][top.position.x] = top.parent;

			if (top.position == finish) break;

			for (auto shift : m_shift) {
				Vec2 to{ shift + top.position };
				Tile type{ m_map->Get(to) };
				if (IsValid(to) && type != Tile::blocked && type != Tile::mActive ) {
					int cost = Cost(to);
					if (m_cost[to.y][to.x] > m_cost[top.position.y][top.position.x] + cost) {
						m_cost[to.y][to.x] = m_cost[top.position.y][top.position.x] + cost;
						Q.emplace(to, top.position, cost);
					}
				}
			}

		}
	}

	deque<pair<Vec2, int>> GetPath(Vec2 start, Vec2 finish) const noexcept {
		/*
			Point<size_t> step = finish;
			while (step != start) {
				m_path.push_front(step);
				step = m_parents[step.y][step.x];
			}
			m_path.push_front(start);
		*/
		deque<pair<Vec2, int>> path;
		Vec2 step = finish;
		while (step != start) {
			Vec2 parent{ m_parent[step.y][step.x] };
			int cost{ m_cost[step.y][step.x] - m_cost[parent.y][parent.x] };
			path.emplace_front( step, cost / 10 );
			step = m_parent[step.y][step.x];
		}
		return path;
	}

	int GetCost(Vec2 finish) const noexcept {
		return m_cost[finish.y][finish.x];
	}

private:
	struct Data {
		Vec2	position;	//tile position
		Vec2 	parent;		//tile from which we had came
		int		cost;	

		Data() = default;
		Data(Vec2 pos, Vec2 par, int c) :
			position(pos), parent(par), cost(c) {}

		friend bool operator <(const Data& t1, const Data&t2) {
			return t1.cost < t2.cost;
		}
		friend bool operator >(const Data& t1, const Data&t2) {
			return t1.cost > t2.cost;
		}
	};

	using DijkstraPriority = priority_queue<Data, vector<Data>, greater<Data>>;

	void ClearDijkstra() {
		for (int i = 0; i < Map::SIZE; i++) {
			for (int j = 0; j < Map::SIZE; j++) {
				m_visited[i][j] = false;
				m_parent[i][j] = Vec2{-1,-1};
				m_cost[i][j] = numeric_limits<int>::max();
			}
		}
	}

	array<Vec2, 4> m_shift;
	bool m_visited[Map::SIZE][Map::SIZE];
	Vec2 m_parent[Map::SIZE][Map::SIZE];
	int  m_cost[Map::SIZE][Map::SIZE];

	Map* m_map;
	UnitManager* m_uManager;
	BuildingManager* m_bManager;
};

class Commander {
public:	
	Commander(Data *data) :
		m_data(data),
		m_search(&data->m_map,&data->m_uManager, &data->m_bManager),
		m_bridges(&data->m_map)
	{
	}

	void UpdateEnemyBridge() {
		m_eBridges = m_bridges.GetBridges(Tile::eActive);
	}
	void UpdateMyBridge() {
		m_mBridges = m_bridges.GetBridges(Tile::mActive);
	}

	void UpdateBridges() {
		UpdateEnemyBridge();
		UpdateMyBridge();
	}

	void Update() {
		m_mHQ = Vec2{ 0,0 }, m_eHQ = Vec2{ 11,11 };
		if (m_data->m_map.Get(Vec2{ 0,0 }) == Tile::eActive)
			swap(m_mHQ, m_eHQ);

		assert(m_eBridges.empty());
		assert(m_mBridges.empty());

		this->UpdateBridges();
	}

	void Clear() noexcept {
		m_takenPositions.clear();
		m_answer.clear();
		m_mBridges.clear();
		m_eBridges.clear();
		m_search.Clear();
	}
	// invalid pair {-1,-1}
	pair<int, int> MinMaxLevelAround(Vec2 p, Tile type) const noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		auto& map{ m_data->m_map };
		int mn = 1000, mx = -1;
		for (auto& sh : shift) {
			auto neighbor{ sh + p };
			if (IsValid(neighbor) && map.Get(neighbor) == type) {
				auto optUnit = m_data->m_uManager.GetUnitAt(neighbor);
				if (optUnit.has_value()) {
					mn = min (optUnit.value().m_level, mn);
					mx = max (optUnit.value().m_level, mx);
				}
			}
		}
		if (mn == 1000 && mx == -1) { mn = -1, mx = -1; }
		return { mn, mx };
	}
	bool CanCreateUnit(bool isMe, int level, int expectedIncome ) const noexcept {
		auto& player = isMe ? m_data->m_me : m_data->m_enemy;
		return player.CanCreateUnit(level, expectedIncome);
	}
	// O (5 * (size(buildings) + size(units)) ): max possible: 144*5 =   720
	optional<Vec2> GetPositionForTowerAround(Vec2 p) const noexcept{
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		auto& map{ m_data->m_map };
		auto& uManager{ m_data->m_uManager };
		auto NoEnemyLevelThree = [&uManager](Vec2 v) {
			auto opt{ uManager.GetUnitAt(v) };
			return !(opt.has_value() && opt->m_level == 3 && !opt->IsMy());

		};
		for (auto& sh : shift) {
			auto neighbor{ sh + p };
			if (IsValid(neighbor)
				&& map.Get(neighbor) == Tile::mInactive 
				&& !m_data->m_bManager.GetBuildingAt(neighbor).has_value()
				&& !uManager.GetUnitAt(neighbor).has_value()
			) {
				bool isSafe{ this->AllNeighbors<decltype(NoEnemyLevelThree)>(neighbor, NoEnemyLevelThree).empty() };

				// if there aren't any enemy units level 3 around!
				if(isSafe)
					return make_optional(neighbor);
			}
		}
		if (!m_data->m_bManager.GetBuildingAt(p).has_value()
			&& !m_data->m_uManager.GetUnitAt(p).has_value())
		{ // it's already checked for @p that htere is no level 3 enemy unit around!
			return make_optional(p);
		}
		return nullopt;
	}
	optional<Vec2> GetInactiveNeighbor(bool isMy, Vec2 p) const noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		auto& map{ m_data->m_map };
		for (auto& sh : shift) {
			auto neighbor{ sh + p };
			if (IsValid(neighbor) && map.Get(neighbor) == (isMy? Tile::mInactive: Tile::eInactive)) {
				return make_optional(neighbor);
			}
		}
		return nullopt;
	}
	bool HasActiveEnemyNeighbor(Vec2 p) const noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		auto& map{ m_data->m_map };
		for (auto& sh : shift) {
			auto neighbor{ sh + p };
			if (IsValid(neighbor) && map.Get(neighbor) == Tile::eActive ) {
				return true;
			}
		}
		return false;
	}
	bool IsBridge(Vec2 p, Tile type) const noexcept {
		auto& bridges{ ( type == Tile::eActive ? m_eBridges : m_mBridges) };
		return (find_if(bridges.begin(), bridges.end(), [&p](auto edge) {
				return p == edge.second || p == edge.first;
		}) != bridges.end());
	
	}
	bool IsTower(Vec2 p) const noexcept {
		auto& buildings{ m_data->m_bManager.m_buildings };
		return (find_if(buildings.begin(), buildings.end(), [&p](auto& b) {
			return (b.IsTower() && p == b.m_pos);
		}) != buildings.end());
	}
	bool IsProtected(Vec2 p) const noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };

		auto& map{ m_data->m_map };
		Tile ty{ map.Get(p) };
		auto& buildings{ m_data->m_bManager.m_buildings };

		if (find_if(buildings.begin(), buildings.end(), [&p,&shift,&map, &ty](auto& b) {
			if (b.IsTower() && ty == map.Get(b.m_pos)) {
				if (b.m_pos == p) return true;

				for (auto& sh : shift) {
					auto towerNeighbor { sh + b.m_pos };
					if (towerNeighbor == p)
					{ // tower and neighbor has the same owner
						return true;
					}
				}
			}
			return false;
		}) != buildings.end())
		{
			return true;
		}
		return false;
	}
	bool IsMineSpot(Vec2 p) const noexcept {
		const auto& spots{ m_data->m_bManager.m_mines };
		return (find_if(spots.cbegin(), spots.cend(), [&p](const auto& spot) {
			return spot.m_pos == p;
		}) != spots.cend());
	}
	bool IsMine(Vec2 p) const noexcept {
		const auto& buildings{ m_data->m_bManager.m_buildings };
		return (find_if(buildings.cbegin(), buildings.cend(), [&p](auto& b) {
			return b.IsMine() && b.m_pos == p;
		}) != buildings.cend());
	}
	template <class Pred>
	vector<Vec2> AllNeighbors(Vec2 center, Pred pred) const noexcept {
		static_assert(is_invocable_v<Pred, Vec2>, "Can't invoce predicate");
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		vector<Vec2> neighbors;
		neighbors.reserve(4);
		auto& map{ m_data->m_map };
		for (auto& sh : shift) {
			auto neighbor{ sh + center };
			if (IsValid(neighbor) && pred(neighbor)) {
				neighbors.emplace_back(neighbor);
			}
		}
		return neighbors;
	}
	vector<Vec2> GetMyMineSpots() const noexcept {
		vector<Vec2> spots;
		auto& mines{ m_data->m_bManager.m_mines };
		auto& map{ m_data->m_map };
		for_each(mines.begin(), mines.end(), [&spots,&map](auto&mine) {
			if (map.Get(mine.m_pos) == Tile::mActive)
				spots.emplace_back(mine.m_pos);
		});
		return spots;
	}

	bool CanChainFrom(Vec2 from, Vec2 to, Tile chainnerType) {
		int cost{ m_search.GetCost(to) };
		if( chainnerType == Tile::eActive ) {
			return cost <= m_data->m_enemy.m_gold + m_data->m_enemy.m_income;
		}
		else {
			return cost <= m_data->m_me.m_gold;
		}
	}
	int ScoreMove(Vec2 dest, const Unit& unit) noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		auto& map{ m_data->m_map };
		auto& uManager{ m_data->m_uManager };
		auto& bManager{ m_data->m_bManager };
		auto& buildings{ bManager.m_buildings };
		auto& enemy{ m_data->m_enemy };

		// calculations that used often
		bool hasActiveEnemyNeighbor{ this->HasActiveEnemyNeighbor(dest) };
		auto myInactiveNeighbor{ this->GetInactiveNeighbor(true, dest) };
		auto[eMinLevel, eMaxLevel] = this->MinMaxLevelAround(dest, Tile::eActive);
		auto worth{ sd::costByLevel[unit.m_level - 1] };
		auto AddMyInactiveComponent = [&](int& score) {
			// @pos is always valid
			auto pred = [&map](Vec2 pos) {
				return map.Get(pos) == Tile::mInactive;
			};
			auto calc = [&buildings, &bManager](Vec2 pos)
			{ // need a sum: visit number and buildings cost!
				auto it = find_if(buildings.begin(), buildings.end(), [&pos](auto&b) {
					return b.m_pos == pos;
				});
				int score{ 0 };
				if (it == buildings.end()) {
					score = sd::inactiveTileScore;
				}
				else if (it->m_type == BType::Tower) {
					score = sd::towerCost + sd::inactiveTileScore;
				}
				else if (it->m_type == BType::Mine) {
					score = sd::minMineCost + 4 * bManager.Count(BType::Mine, false) + sd::inactiveTileScore;
				}
				return score;
			};
			m_search.Clear();
			m_search.Dfs<decltype(pred), decltype(calc)>(myInactiveNeighbor.value(), score, pred, calc);
			cerr << "Find |CC| with size of " << score << " at " << dest << endl;
		};

		const int mx{ 1000 };
		const int mn{ -mx };


		Tile destType{ map.Get(dest) };
		int score{ 0 };

		if (dest == m_mHQ) return mn;
		if (dest == m_eHQ) return mx;
		
		switch (destType) {
		case Tile::blocked: {
			cerr << "Trying to score blocked tile: " << dest << endl;
		}; break;
		case Tile::neutral: {
			score = sd::defaultScore;
			if (hasActiveEnemyNeighbor && this->CanCreateUnit(false, 3, 1) && unit.m_level == 3)
			{ // otherwise all units are cheap and I don't care about low level one
				cerr << "Can't risk level 3 unit!" << endl;
				score = mn;
			}
		}; break;
		case Tile::mActive: {
			bool isEmpty{ true };
			isEmpty &= !uManager.GetUnitAt(dest).has_value();
			if(isEmpty) isEmpty &= !bManager.GetBuildingAt(dest).has_value();
			if (unit.m_pos == dest || isEmpty) {
				bool isBridge{ this->IsBridge(dest, Tile::mActive) };
				if (hasActiveEnemyNeighbor && isBridge) {
					bool isDangerous{ this->CanCreateUnit(false, min(unit.m_level + 1, 3), 1) };
					if (isDangerous || eMaxLevel > unit.m_level || eMaxLevel == 3) {
						score = mn;
						if (unit.m_pos == dest) 
							score = mn + 1; // {-999} to not move if all score around are @mn {-1000}
						cerr << "Expect to train|build def at " << dest << endl;
					}
					else { 
						// CC ( units +  buildings + visits) which can be lost with destroyed bridge!
						m_search.Clear();
						score = m_search.GetScoreAfterBridge(m_mHQ, dest, Tile::mActive);
						if (unit.m_pos == dest) score -= sd::costByLevel[unit.m_level - 1]; // don't include unit
						cerr << "Trying to save |CC| with size of " << score << " at " << dest << endl;
					}
				}
			}
		}; break;
		case Tile::mInactive: {
			cerr << "Error! Trying to create on my inactive tile : " << dest << endl;
		}; break;
		case Tile::eActive: {
			bool isBridge{ this->IsBridge(dest, Tile::eActive) };
			//cerr << dest << " is bridge: " << boolalpha << isBridge << endl;
			auto optUnit{ uManager.GetUnitAt(dest) };
			auto optBuilding{ bManager.GetBuildingAt(dest) };
			auto addForBridge = [&]() {
				auto deal{ 0 };
				// CC ( units +  buildings + visits) which can be lost with destroyed bridge!
				m_search.Clear();
				deal = m_search.GetScoreAfterBridge(m_eHQ, dest, Tile::eActive);
				score += deal;
			};
			if (this->IsProtected(dest)) // O(buildings.size() * shift.size())
			{ // only unit level 3 can advance
				if (unit.m_level == 3) {
					if (myInactiveNeighbor.has_value()) {
						AddMyInactiveComponent(score);
					}
					if (isBridge) {
						addForBridge();
						cerr << "Trying to break enemy bridge with size of CC with : " 
							<< score << " score for " << dest << endl;
						if (score > 0) {
							cerr << "For better life! Deal|lose: " << score << "|" << worth << endl;
						}
						else {
							cerr << "We're losing: " << score << " < " << worth << endl;
						}
					}
					else {
						bool isDangerous{ enemy.CanCreateUnit(3, 1) };
						score += sd::activeTileScore;
						if (isDangerous || eMaxLevel == unit.m_level) {
							if (eMaxLevel < 3)  score += sd::costByLevel[3-1] / 2; // he will have to build LEVEL 3 unit! So it' quite good!
							else score -= worth;
						}
						
						if (optBuilding.has_value()) score += optBuilding.value().IsMine() ? sd::minMineCost : sd::towerCost;
						else if (optUnit.has_value()) score += sd::costByLevel[optUnit.value().m_level - 1];
					}
				}
				else {
					score = mn; // invalid command
				}
			}
			else { 
				if(this->IsTower(dest))  {
					if (unit.m_level == 3) {
						score = sd::towerCost;
						if (myInactiveNeighbor.has_value()) {
							AddMyInactiveComponent(score);
						}
						if (isBridge) {
							addForBridge();
							score -= sd::towerCost; // to not calculate tower 2 times
							cerr << "Trying to break enemy bridge with tower: " 
								 << score << " score for " << dest << endl;
						}
					}
					else { // can't do anything to tower wiht low level unit
						score = mn;
					}
				}
				else {
					int	minReqLevelToKill{ min(unit.m_level + 1, 3) }; // level requirement to kill my unit
					bool canCreateBetterUnit{ enemy.CanCreateUnit(minReqLevelToKill, 1) };
					bool canKillMyUnit{ eMaxLevel > unit.m_level || eMaxLevel == 3 };

					if (myInactiveNeighbor.has_value()) {
						AddMyInactiveComponent(score);
					}

					if (optUnit.has_value()) {
						auto enemyLevel{ optUnit->m_level };
						
						if (enemyLevel < unit.m_level || unit.m_level == 3) {
							score = sd::activeTileScore;
							if (canKillMyUnit) {
								score -= worth;
							}
							else if (canCreateBetterUnit) {// we force to create unit with greater strength
								score += sd::costByLevel[minReqLevelToKill - 1] / 2; 
							}
							
							if (isBridge) {
								addForBridge();
								score -= sd::activeTileScore; //is calc again in @addForBridge();
								cerr << "Trying to break enemy bridge: "
									<< score << " score for " << dest << endl;
							}
							else {
								score += sd::costByLevel[enemyLevel - 1];
							}
						}
						else {
							score = mn;
						}
					}
					else {
						score += sd::activeTileScore;
						if (unit.m_level == 1) { // not important units
							//do nothing
						}
						else if (canKillMyUnit) {
							score -= worth;
						} //TODO: smth shady is going on there! CHANGE!
						else if (canCreateBetterUnit) {// we force to create unit with greater strength
							score += sd::costByLevel[minReqLevelToKill - 1] / 2;
						}
						if (isBridge) {
							addForBridge();
							score -= sd::activeTileScore; //already added in AddForBridge
							cerr << "Trying to break enemy bridge: "
								<< score << " score for " << dest << endl;
						}
						else if (this->IsMine(dest)) {
							score += sd::minMineCost;
						}
					}
				}
			}
		}; break;
		//used when map hasn't updated yet (not it should by hand but not sre it works good)
		case Tile::eInactive: {
			score = sd::inactiveTileScore;
			if(myInactiveNeighbor.has_value())
			{ // calculate CC size:
				AddMyInactiveComponent(score);
			}
		}; break;
		default:{
			cerr << "Trying to score undefined tile: " << dest << endl;
		}; break;
		}

		return score;
	}

	/* MOVE:
	1. foreach my unit 
	2. give score to my neighbors and tile on already occupied tile (maybe it's better to not move):
	3. choose with max score
	4. add to answer array
	*/
	void Move() {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {0, -1}, {0, 1}, {1, 0} };

		auto& map{ m_data->m_map };
		auto& uManager{ m_data->m_uManager };
		auto& bManager{ m_data->m_bManager };
		auto& units{ uManager.m_units };

		std::random_device rd;
		std::mt19937 g(rd());
		shuffle(units.begin(), units.end(),g);

		cerr << "All units: " << endl;
		for_each(units.begin(), units.end(), [](auto&u) {cerr << u.m_id <<" "<<u.m_pos <<"; "; });
		cerr << endl;

		for (auto& unit : units) {
			if (unit.IsMy()) {
				vector<pair<Vec2, int>> targets;
				targets.reserve(5); // max possible number of neighbors + occupied tile
				for (auto& sh : shift) {
					auto neighbor{ unit.m_pos + sh };
					
					if (!IsValid(neighbor)) continue;
					auto occupant { uManager.GetUnitAt(neighbor) };
					auto building{ bManager.GetBuildingAt(neighbor) };
					bool isOccupiedByEnemy{ occupant.has_value() && !occupant->IsMy()};
					auto isOccupiedByMyBuilding{ building.has_value() && building->IsMy() };

					if ( map.Get(neighbor) != Tile::blocked && //not blocked
						(!occupant.has_value() || isOccupiedByEnemy) && //nobody or enemy
						!isOccupiedByMyBuilding
						//	m_takenPositions.count(neighbor) == 0 ) // isn't occupied  
					) { // calculate score:
						targets.emplace_back(neighbor, this->ScoreMove(neighbor, unit));
					}
				}
				//calculate score if we won't move
				auto stay{ make_pair(unit.m_pos, this->ScoreMove(unit.m_pos, unit)) };

				targets.emplace_back(stay);
				
				cerr << "id: " << unit.m_id << ",targets:" << endl;
				for_each(targets.begin(), targets.end(), [](auto&p) {
					cerr << p.first << " " << p.second <<"; ";
				});
				cerr << endl;
				
				auto bestTarget = max_element(targets.begin(), targets.end(), [this](auto& l, auto& r) {
					if (l.second == r.second) {
						return l.first.Distanse(m_eHQ) > r.first.Distanse(m_eHQ);
					}
					return l.second < r.second;
				});

				if (!(bestTarget->first == unit.m_pos)) {
					m_answer.emplace_back(commands::Move(unit.m_id, bestTarget->first));
					if (this->IsBridge(bestTarget->first, Tile::eActive)) 
					{ // cut of connected component
						m_search.Clear();
						m_search.ChangeTilesAfterTheBridge(m_eHQ, bestTarget->first, Tile::eActive, Tile::eInactive);
					}
					//CAN BE ERROR IF INVALID TRAINING (THERE IS UNIT WITH LEVEL > MY LEVEL)
					// i.g. all scores are -1000, will it  choose the tile close to headquaters?? 
					map.m_map[bestTarget->first.y][bestTarget->first.x] = Tile::mActive;
					//m_takenPositions.insert(bestTarget->first);
					unit.m_pos = bestTarget->first; // update position
				}
			}
		}
	}

	void DefendFromChainAttack() {
		auto& uManager{ m_data->m_uManager };
		auto& bManager{ m_data->m_bManager };
		auto& me{ m_data->m_me };
		auto& map{ m_data->m_map };

	//	for()
	}
	void TryChainAttack() {
		auto tiles{ m_search.GetOutline(Tile::mActive) };
		cerr << "Outline: ";
		for (auto tile : tiles) {
			cerr << tile.first << " ";
		}
		cerr << endl;
		for (auto tile : tiles) {
			m_search.FindPath(tile.first, m_eHQ);
			int cost{ m_search.GetCost(m_eHQ) };
			bool canChain{ cost <= m_data->m_me.m_gold };
			cerr << "\tCost from " << tile.first << " to " << m_eHQ << " is " << cost << " ? " << m_data->m_me.m_gold << endl;
			if (canChain)
			{
				auto path{ m_search.GetPath(tile.first, m_eHQ) };
				for (auto [step, level] : path) {
					if (!m_data->m_me.CanCreateUnit(level,0)) {
						cerr<<"ERROR! CAN'T CREATE UNIT!"<<endl;
					}
					m_data->m_me.CreateUnit(level, 0);
					m_answer.emplace_back(commands::Train(level, step));
				}
				break;
			}
		}
	}

	void ReinforceBoarderline() {
		auto& uManager{ m_data->m_uManager };
		auto& bManager{ m_data->m_bManager };
		auto& me{ m_data->m_me };
		auto& map{ m_data->m_map };

		// define tiles which need to reinforce:
		auto weakTiles = m_search.GetOutline(Tile::mActive);

		cerr << "\t1st Weaklings left on the outline: ";
		for (auto[pos, score] : weakTiles) {
			cerr << pos << " ";
		}
		cerr << endl;

		auto Filter = [&](auto & p) {
			auto pos{ p.first };
			// kick protected
			if (this->IsProtected(pos)) return true;
			// kick tiles without Tile::eActive around
			if (!this->HasActiveEnemyNeighbor(pos)) return true;
			// kick with unit 2,3
			auto optUnit{ uManager.GetUnitAt(pos) };
			if (optUnit.has_value() && optUnit->m_level > 1) return true;
			// kick tiles with enemy Unit 3 as neighbor
			//if (this->MinMaxLevelAround(pos, Tile::eActive).second == 3) return true;
			return ( this->MinMaxLevelAround(pos, Tile::eActive).second == 3);
		};
		weakTiles.erase( remove_if(weakTiles.begin(), weakTiles.end(), Filter),  weakTiles.end());

		bool isWeak[Map::SIZE][Map::SIZE];
		for (int i = 0; i < Map::SIZE; i++)
			for (int j = 0; j < Map::SIZE; j++)
				isWeak[i][j] = false;

		auto IsSpotForTower = [&](Vec2 pos) 
		{ // is our active tile + not mine spot + wasn't added + without unit
			if (map.Get(pos) != Tile::mActive || this->IsMineSpot(pos)) return false;
			if (isWeak[pos.y][pos.x]) return false; // already added or will be added from @weakTiles
			return ( !uManager.GetUnitAt(pos).has_value() );
		};

		for (const auto&[tile, score] : weakTiles) {
			isWeak[tile.y][tile.x] = true;
		}

		vector<Vec2> possibleTowerPositions;
		for (const auto&[tile, score] : weakTiles) {
			auto optUnit{ uManager.GetUnitAt(tile) };
			if (!optUnit.has_value() && !this->IsMineSpot(tile)) {
				possibleTowerPositions.emplace_back(tile);
			}
			// add all[empty mActive] positions around:
			auto neighbors = this->AllNeighbors(tile, IsSpotForTower);
			for (auto neighbor : neighbors) {
				possibleTowerPositions.emplace_back(neighbor);
			}
		}
		
		auto IsWeakTile = [&isWeak](Vec2 tile) {
			return isWeak[tile.y][tile.x];
		};

		array<Vec2, 5> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {0, 0} };

		while (me.CanCreateBuilding(sd::towerCost) && !possibleTowerPositions.empty())
		{
			int	mxWeaklings = 0;
			Vec2 bestTile {0,0};

			for (Vec2 posForTower : possibleTowerPositions) {
				int weakCount{ (int)this->AllNeighbors(posForTower, IsWeakTile).size() };
				if (isWeak[posForTower.y][posForTower.x]) weakCount++;

				if (this->MinMaxLevelAround(posForTower, Tile::eActive).second == 3) continue;

				if (weakCount == mxWeaklings && bestTile.Distanse(m_mHQ) > posForTower.Distanse(m_mHQ)) 
				{
					mxWeaklings = weakCount;
					bestTile = posForTower;
				}
				else if (weakCount > mxWeaklings) {
					mxWeaklings = weakCount;
					bestTile = posForTower;
				}
			}
		
			if (mxWeaklings < 2) break;
			// mark all neighbors as protected
			for (auto sh : shift) {
				Vec2 neighbor{ sh + bestTile };
				if (IsValid(neighbor)) {
					isWeak[neighbor.y][neighbor.x] = false;
				}
			}
			
			me.CreateBuilding(sd::towerCost, 0);
			m_answer.emplace_back(commands::Build(BType::Tower, bestTile));
			// UPDATE BUILDINGS
			bManager.AddBuilding(0, BType::Tower, bestTile);
			::cerr << "Defend weak by tower at: " << bestTile << endl;
		}

		auto Filter2 = [&](auto & p) {
			auto pos{ p.first };
			if (!isWeak[pos.y][pos.x]) return true;
			// kick with unit
			if (uManager.GetUnitAt(pos).has_value()) return true;
			// kick tiles with enemy Unit 2,3 as neighbor
			//if (this->MinMaxLevelAround(pos, Tile::eActive).second == 3) return true;
			return (this->MinMaxLevelAround(pos, Tile::eActive).second > 1);
		};
		weakTiles.erase(remove_if(weakTiles.begin(), weakTiles.end(), Filter2), weakTiles.end());

		cerr << "Weaklings left on the outline: ";
		for (auto[pos, score] : weakTiles) {
			cerr << pos << " ";
		}
		cerr << endl;

		// train
		while (me.CanCreateUnit(1, 0) && !weakTiles.empty()) {
			Vec2 bestTile{ weakTiles.front().first};

			bool found = false;
			for (auto[pos, score] : weakTiles) {
				if (!isWeak[pos.y][pos.x]) continue;

				if (pos.Distanse(m_mHQ) <= bestTile.Distanse(m_mHQ)) {
					bestTile = pos;
					found = true;
				}
			}
			if (!found) break;

			isWeak[bestTile.y][bestTile.x] = false;

			::cerr << "\tReinforce by unit 1 at " << bestTile << endl;
			me.CreateUnit(1, 0);
			m_answer.emplace_back(commands::Train(1, bestTile));
			// UPDATE UNITS
			uManager.AddUnit(0, -1, 1, bestTile); // -1 is undef id
			map.m_map[bestTile.y][bestTile.x] = Tile::mActive;
		}
	}

	void Train() {
		this->TryChainAttack();
		this->DefendFromChainAttack();
		this->DefendBridges();
		this->ReinforceBoarderline();
		this->AttackEnemy();
	}

	void Build() {
		auto& me{ m_data->m_me };
		auto& uManager{ m_data->m_uManager };
		auto& bManager{ m_data->m_bManager };
		bool isMy{ true };
		auto myMines{ bManager.GetMines(isMy)};

		int mineCost{ static_cast<int>( sd::minMineCost + 4 * myMines.size()) };
		if (!me.CanCreateBuilding(mineCost)) return;

		auto mineSpots{ this->GetMyMineSpots() };
		
		if (me.m_gold < 100 || myMines.size() >= 2) return;

		sort(mineSpots.begin(), mineSpots.end(), [this](const Mine& lsh,const Mine& rsh) {
			return lsh.m_pos.Distanse(m_mHQ) < rsh.m_pos.Distanse(m_mHQ);
		});

		for (auto& mine : mineSpots) {
			if (find_if(myMines.begin(), myMines.end(), [&mine, &uManager](const Building& b) {
				return b.m_pos == mine && !uManager.GetUnitAt(mine).has_value();
			}) == myMines.end()
			) {
				me.CreateBuilding(mineCost, 4);
				m_answer.emplace_back(commands::Build(BType::Mine, mine));
				bManager.AddBuilding(0, BType::Mine, mine);
				break;
			}
		}
	}

	void Print() {
		for (auto&s : m_answer) {
			cout << s;
		}
		if (m_answer.empty()) cout << commands::Wait();
		cout << endl;
	}
private:
	
	// CALL ONLY AFTER MOVE!
	void DefendBridges() {
		auto& uManager{ m_data->m_uManager };
		auto& bManager{ m_data->m_bManager };
		auto& me{ m_data->m_me };
		auto& map{ m_data->m_map };

		vector<pair<int, Vec2>> values(m_mBridges.size());
		
		bool added[Map::SIZE][Map::SIZE];
		for (int i = 0; i < Map::SIZE; i++)
			for (int j = 0; j < Map::SIZE; j++)
				added[i][j] = false;

		for (const auto&[from, to] : m_mBridges) {
			if (!added[from.y][from.x]) {
				// calculate bridge value:
				m_search.Clear();
				values.emplace_back(m_search.GetScoreAfterBridge(m_mHQ, from, Tile::mActive), from);
				added[from.y][from.x] = true;
			}
			if (!added[to.y][to.x]) {
				// calculate bridge value:
				m_search.Clear();
				values.emplace_back(m_search.GetScoreAfterBridge(m_mHQ, to, Tile::mActive), to);
				added[to.y][to.x] = true;
			}
		}
		sort(values.rbegin(), values.rend());

		cerr << "My bridge's worth: [ ";
		for (const auto&[worth, bridge] : values) {
			cerr << "[" << bridge << ": " << worth << "]; ";
		}
		cerr << " ]"<< endl;

		for (const auto&[worth, bridge ]: values)
		{ 
			const int minWorth{ 3 }; // i think it's good to protect at least 2 tiles!
			if (worth < minWorth) {
				cerr << bridge << "with worth: "<< worth <<" is worthless bridge!" << endl;
				break;
			}
			// make clear treats:
			bool hasEnemyAround{ this->HasActiveEnemyNeighbor(bridge) };
			if (!hasEnemyAround) continue;

			auto optUnit{ uManager.GetUnitAt(bridge) };
			int levelOnBridge{ (optUnit.has_value() ? optUnit.value().m_level : 0) };
			bool isProtected { this->IsProtected(bridge) };
			auto[minLevelTreat, maxLevelTreat] = this->MinMaxLevelAround(bridge, Tile::eActive);
	
			if (!isProtected
				&& levelOnBridge < 2
				&& maxLevelTreat != 3
			) {// we can't nullify or it's not worth money!
				auto optTowerPos{ this->GetPositionForTowerAround(bridge) };
				const int minLevelDefender{ 2 };
				if (optTowerPos.has_value() &&
					me.CanCreateBuilding(sd::towerCost) &&
					!bManager.IsMineSpot(*optTowerPos) && // not a mine spot
					worth > sd::towerCost
				) {
					me.CreateBuilding(sd::towerCost, 0);
					m_takenPositions.insert(optTowerPos.value());
					m_answer.emplace_back(commands::Build(BType::Tower, optTowerPos.value()));
					// UPDATE BUILDINGS
					bManager.AddBuilding(0, BType::Tower, optTowerPos.value());
					isProtected = true;
					cerr << "Create bridge-defending building: " << optTowerPos.value() << endl;
				}
				else if (!levelOnBridge &&
					!bManager.GetBuildingAt(bridge).has_value() &&
					me.CanCreateUnit(minLevelDefender, 0) &&
					worth > sd::costByLevel[minLevelDefender - 1] * 2
				) { // can't create a tower and no unit/building on the bridge
					me.CreateUnit(minLevelDefender, 0);
					m_answer.emplace_back(commands::Train(minLevelDefender, bridge));
					// UPDATE UNITS
					levelOnBridge = minLevelDefender;
					uManager.AddUnit(0, -1, minLevelDefender, bridge); // -1 is undef id
					cerr << "Create bridge-defending unit: " << bridge << endl;
				}
			}

			// level of attacker that can destroy my bridge
			minLevelTreat = ( isProtected ? 3 : min(levelOnBridge + 1, 3) );
			//can be oprimized using lambda as modifier for @trainLevels vector below
			auto treats{ this->AllNeighbors(bridge, [&map, &uManager,&minLevelTreat](Vec2 pos) {
					if (map.Get(pos) == Tile::eInactive) return false;
					auto optUnit = uManager.GetUnitAt(pos);
					bool hasTreatningEnemy = (optUnit.has_value()
						&& optUnit->m_level >= minLevelTreat
						&& !optUnit->IsMy()); // 1 is id of enemy
					return  (hasTreatningEnemy || map.Get(pos) == Tile::eActive);
				})
			};
			int trainingCost{ 0 };
			vector<int> trainLevels;
			trainLevels.reserve(treats.size());
			for (Vec2 p : treats) {
				auto enemyAround = uManager.GetUnitAt(p);
				auto level{ enemyAround.has_value() ? enemyAround->m_level : 0 };
				int needLevelForKill{ this->IsProtected(p) ? 3 : min(level+1, 3) };
				trainLevels.emplace_back(needLevelForKill);
				trainingCost += sd::costByLevel[needLevelForKill-1];
			}
			
		if (me.CanCreateUnits(trainLevels) 
			&& (worth > trainingCost * 2) 
			&& !trainLevels.empty()
			) { // we can nullify all treats by training!
				cerr << "Remove treat : ";
				for (size_t i = 0; i < trainLevels.size(); i++) {
					cerr << treats[i] << "; ";
					if (trainLevels[i] > 1) {
						// remove enemy unit from map
						uManager.MarkUnitForRemove(treats[i]);//if there is any
					}
					me.CreateUnit(trainLevels[i], 1);
					m_answer.emplace_back(commands::Train(trainLevels[i], treats[i]));
					// UPDATE MAP & UNITS
					map.m_map[treats[i].y][treats[i].x] = Tile::mActive;
					uManager.AddUnit(0, -1, trainLevels[i], treats[i]); // -1 is undef id
				}
				cerr << endl;
				uManager.RemoveMarkedUnits();
			}
		}
	}
	// CALL ONLY AFTER DEFEND!
	void AttackEnemy() {
		//declaration:
		auto& uManager{ m_data->m_uManager };
		auto& bManager{ m_data->m_bManager };
		auto& me{ m_data->m_me };
		auto& map{ m_data->m_map };
		array<Vec2, 4> shift{ Vec2{-1, 0}, {0, -1}, {0, 1}, {1, 0} };
		//get tiles on the boarder
		auto tiles{ m_search.GetBoarderTiles(Tile::mActive) };

		bool added[Map::SIZE][Map::SIZE];
		for (int i = 0; i < Map::SIZE; i++)
			for (int j = 0; j < Map::SIZE; j++)
				added[i][j] = false;
		// sort enemy bridges by score, keep position
		vector<pair<int, Vec2>> bridges;
		bridges.reserve(m_eBridges.size());
		for (auto [from, to] : m_eBridges) {
			if (!added[from.y][from.x]) {
				// calculate bridge value:
				m_search.Clear();
				bridges.emplace_back(m_search.GetScoreAfterBridge(m_eHQ, from, Tile::eActive), from);
				added[from.y][from.x] = true;
			}
			if (!added[to.y][to.x]) {
				// calculate bridge value:
				m_search.Clear();
				bridges.emplace_back(m_search.GetScoreAfterBridge(m_eHQ, to, Tile::eActive), to);
				added[to.y][to.x] = true;
			}
		}
		sort(bridges.rbegin(), bridges.rend());

		cerr << "Enemy bridge's worth: [ ";
		for (const auto&[worth, bridge] : bridges) {
			cerr << "[" << bridge << ": " << worth << "];  ";
		}
		cerr << endl;

		// Find all intersections (bridges under attack) where:
		// bridges INTERSECT tilesOnBoarder
		vector<pair<int , Vec2>> bridgesUnderAttack;
		bridgesUnderAttack.reserve(10);
		for (auto [worth, bridge] : bridges) {
			for (auto [boarderTile, score] : tiles) {
				if (bridge == boarderTile) { // we can attack bridge
					bridgesUnderAttack.emplace_back(worth, bridge);
					break;
				}
			}
		}
		// solve bridges that we can attack in online style (update map & unit & building data after each case):
		// NOTE: It's just erase connected component after the bridge.
		// It doesn't update/search for new bridges after removing current one
		vector<Vec2> solved;
		for (const auto& [worth, bridge] : bridgesUnderAttack) {
			// i think it's important to erase bridges where |CC| >= minWorth
			// it also can be unit
			const int minWorth{ 3 }; 
			if (worth < minWorth) {
				cerr << bridge << " is worthless enemy bridge!" << endl;
				break;
			}
			if (map.Get(bridge) == Tile::eInactive) {
				solved.emplace_back(bridge);
				continue;
			}

			auto optUnit{ uManager.GetUnitAt(bridge) };
			int levelOnBridge{ (optUnit.has_value() ? optUnit.value().m_level : 0) };
			bool isProtected{ this->IsProtected(bridge) };
			int attackerLevel{ isProtected ? 3 : min(levelOnBridge + 1, 3) };

			int cost{ sd::costByLevel[attackerLevel - 1] };
			int managableGold{ 0 /* sd::costByLevel[0] - minWorth*/ }; // if can solve with lvl 1 UNit
			
			bool canUseLevel[3] = { ( worth > 3 ), (worth > cost ), ( worth > cost * 2) };

			if ( canUseLevel[attackerLevel - 1] && me.CanCreateUnit(attackerLevel, 1)) {
				solved.emplace_back(bridge);
				me.CreateUnit(attackerLevel, 1);
				m_answer.emplace_back(commands::Train(attackerLevel, bridge));
				// UPDATE MAP & UNITS
				map.m_map[bridge.y][bridge.x] = Tile::mActive;
				uManager.AddUnit(0, -1, attackerLevel, bridge); // -1 is undef id
				cerr << "Attack bridge by unit: " << bridge << endl;
				// MAKE INACTIVE EVERY TILE AFTER BRIDGE!
				m_search.Clear();
				m_search.ChangeTilesAfterTheBridge(m_eHQ, bridge, Tile::eActive, Tile::eInactive);
			}
		}
//STEP 2: 
		// ALSO ONLINE:
		int hasTerritory{ true };
		int expandTeamLevel{ 1 };
		bool isMarked[Map::SIZE][Map::SIZE];

		while (hasTerritory) {
			if (!me.CanCreateUnit(expandTeamLevel, 1)) break;

			tiles = m_search.GetBoarderTiles(Tile::mActive);
			// MARK FOR LEVEL 1
			// mark next possible steps of my units for spreading:
			for (int i = 0; i < Map::SIZE; i++) {
				for (int j = 0; j < Map::SIZE; j++) {
					isMarked[i][j] = false;
				}
			}
			//mark neutral tiles for the next step of existing units to avoid stucking!
			for (const auto& unit : uManager.m_units) {
				if (unit.IsMy()) {
					for (auto sh : shift) {
						Vec2 neighbor{ unit.m_pos + sh };
						if (IsValid(neighbor) && map.Get(neighbor) == Tile::neutral) {
							isMarked[neighbor.y][neighbor.x] = true;
							break; // mark and go mark for the next unit
						}
					}
				}
			}
			
			auto TileFilter = [&](const auto& p) {
				return (isMarked[p.first.y][p.first.x] ||
						this->IsProtected(p.first) ||
						map.Get(p.first) == Tile::blocked ||
						uManager.GetUnitAt(p.first).has_value()
				);
			};
			// filter:
			tiles.erase(
				remove_if(tiles.begin(), tiles.end(), TileFilter),
				tiles.end()
			);

			if (tiles.empty()) break;
			// score all bridges
			int bridgeScores[Map::SIZE][Map::SIZE];
			for (int i = 0; i < Map::SIZE; i++) {
				for (int j = 0; j < Map::SIZE; j++) {
					bridgeScores[i][j] = 0;
				}
			}
			this->UpdateEnemyBridge();
			for (auto[from, to] : m_eBridges) {
				if (!bridgeScores[from.y][from.x]) {
					// calculate bridge value:
					m_search.Clear();
					bridgeScores[from.y][from.x] = m_search.GetScoreAfterBridge(m_eHQ, from, Tile::eActive);
				}
				if (!bridgeScores[to.y][to.x]) {
					// calculate bridge value:
					m_search.Clear();
					bridgeScores[to.y][to.x] = m_search.GetScoreAfterBridge(m_eHQ, to, Tile::eActive);
				}
			}

			// score expansion:
			auto bestTile{ tiles.front().first };
			auto maxScore{ tiles.front().second };
			for (auto&[tile, score] : tiles) 
			{
				auto type = map.Get(tile);

				if (bridgeScores[tile.y][tile.x]) {
					score = bridgeScores[tile.y][tile.x];
				}
				else if (type == Tile::eActive)
					score = sd::activeTileScore + (this->IsMine(tile) ? sd::minMineCost : 0);
				else if (type == Tile::eInactive)
					score = sd::inactiveTileScore;
				else
					score = sd::defaultScore;
				// look for inactive component around our tile
				// how much we will get if activate?
				for (auto&sh : shift) {
					auto neighbor{ sh + tile };
					if (!IsValid(neighbor) || map.Get(neighbor) != Tile::mInactive) continue;
					auto pred = [&map](Vec2 p) {
						return (map.Get(p) == Tile::mInactive);
					};
					auto calc = [&bManager](Vec2 p) {
						auto optBuilding{ bManager.GetBuildingAt(p) };
						int res{ 0 };
						if (optBuilding.has_value()) {
							res += (optBuilding.value().IsTower() ? sd::towerCost : sd::minMineCost);
						}
						return res;
					};
					m_search.Clear();
					m_search.Dfs<decltype(pred), decltype(calc)>(neighbor, score, pred, calc);
				}

				if (maxScore == score && bestTile.Distanse(m_eHQ) > tile.Distanse(m_eHQ)) 
				{
					maxScore = score;
					bestTile = tile;
				}
				else if (maxScore < score) {
					maxScore = score;
					bestTile = tile;
				}
			}
			
			// TRAIN LEVEL 1 UNITS
			// to expand territory
		
			::cerr << "\t create unit 1 at " << bestTile << endl;
			me.CreateUnit(expandTeamLevel, 1);
			m_answer.emplace_back(commands::Train(expandTeamLevel, bestTile));
			// UPDATE UNITS
			uManager.AddUnit(0, -1, expandTeamLevel, bestTile); // -1 is undef id
			map.m_map[bestTile.y][bestTile.x] = Tile::mActive;
			hasTerritory = true;
		}
// STEP 3:
		hasTerritory = true;
		const int negative = -10'000'000;
		while (hasTerritory) {
			hasTerritory = false;
			tiles = m_search.GetBoarderTiles(Tile::mActive);
			tuple<int, Vec2, int> bestResult { negative, Vec2{0,0}, 0 };
			// precalculation of all tiles:
			this->UpdateEnemyBridge();
			// score all bridges
			int bridgeScores[Map::SIZE][Map::SIZE];
			for (int i = 0; i < Map::SIZE; i++) {
				for (int j = 0; j < Map::SIZE; j++) {
					bridgeScores[i][j] = 0;
				}
			}
			for (auto[from, to] : m_eBridges) {
				if (!bridgeScores[from.y][from.x]) {
					// calculate bridge value:
					m_search.Clear();
					bridgeScores[from.y][from.x] = m_search.GetScoreAfterBridge(m_eHQ, from, Tile::eActive);
				}
				if (!bridgeScores[to.y][to.x]) {
					// calculate bridge value:
					m_search.Clear();
					bridgeScores[to.y][to.x] = m_search.GetScoreAfterBridge(m_eHQ, to, Tile::eActive);
				}
			}
			for (auto& [tile, score] : tiles)
			{ // check whether we already have solved this one (if it was bridge
				score = 0;
				if (!solved.empty()) {
					auto solvedIt = find_if(solved.begin(), solved.end(), [&tile](auto&pos) { return pos == tile; });
					if (solvedIt != solved.end()) {
					// to not chose this tile for the training command
						continue;
					}
				}
				if (m_eHQ == tile) {
					bestResult = { sd::mxScore, tile, sd::costByLevel[0] };
					break;
				}

				auto optEnemy{ uManager.GetUnitAt(tile) };
				auto optBuilding{ bManager.GetBuildingAt(tile) };
				bool isProtected{ this->IsProtected(tile) };
				int enemyLevel{ optEnemy.has_value() ? optEnemy->m_level : 0 };
				int attackerLevel{ isProtected ? 3 : min(enemyLevel + 1, 3) };

				// ignore level 3 unit as attacker:
				// to have opportunity to attack
				bool isEnough{ (me.m_gold > 45 && me.m_income > 30) || me.m_gold >= 90 };
				if (attackerLevel == 3 && !isEnough) continue; // don't choose level 3
				if (!me.CanCreateUnit(attackerLevel, 1)) continue;
				
				// score all other cases:
				score = sd::defaultScore;
				if (map.Get(tile) == Tile::eActive) {
					score = sd::activeTileScore;
				}
				else if (map.Get(tile) == Tile::eInactive) {
					score = sd::inactiveTileScore;
				}

				if (map.Get(tile) != Tile::neutral) {
					if (optBuilding.has_value()) {
						score += (optBuilding.value().IsTower() ? sd::towerCost : sd::minMineCost);
					}
					else if (enemyLevel) {
						score += sd::costByLevel[enemyLevel - 1];
					}
				}

				//check if it's bridge
				if (bridgeScores[tile.y][tile.x]) {
					int bridgeWorth{ bridgeScores[tile.y][tile.x] - sd::costByLevel[attackerLevel - 1] };
					int best{ get<0>(bestResult) - get<2>(bestResult) };// score - cost
					if (bridgeWorth > best) {
						score = bridgeScores[tile.y][tile.x];
						//bestResult = {score, tile, sd::costByLevel[attackerLevel - 1] };
					}
				}
				
				// look for inactive component around our tile
				// how much we will get if activate?
				for (auto&sh : shift) {
					auto neighbor{ sh + tile };
					if (!IsValid(neighbor) || map.Get(neighbor) != Tile::mInactive) continue;
					auto pred = [&map](Vec2 p) {
						return (map.Get(p) == Tile::mInactive);
					};
					auto calc = [&bManager](Vec2 p) {
						auto optBuilding{ bManager.GetBuildingAt(p) };
						int res{ 0 };
						if (optBuilding.has_value()) {
							res += (optBuilding.value().IsTower() ? sd::towerCost : sd::minMineCost);
						}
						return res;
					};
					m_search.Clear();
					m_search.Dfs<decltype(pred), decltype(calc)>(neighbor, score, pred, calc);
				}

				int deltaCur{ score - sd::costByLevel[attackerLevel - 1] };
				int deltaBest{ get<0>(bestResult) - get<2>(bestResult) };

				if (deltaCur == deltaBest && get<1>(bestResult).Distanse(m_eHQ) > tile.Distanse(m_eHQ)) 
				{
					bestResult = { score, tile, sd::costByLevel[attackerLevel - 1] };
				}
				else if (deltaCur > deltaBest) {
					bestResult = { score, tile, sd::costByLevel[attackerLevel - 1] };
				}
			}

			// Start Attack!
			cerr << "Best target for attack:(score|pos|cost): ";
			cerr << "[" << get<0>(bestResult) << " " << get<1>(bestResult) << " " << get<2>(bestResult) << "];" << endl;
			int level{ 1 };
			for (int i = 1; i <= 3; i++) {
				int x{ sd::costByLevel[i - 1] };
				if (get<2>(bestResult) == x) {
					level = i;
					break;
				}
			}
		
			if (me.CanCreateUnit(level, 1) && get<0>(bestResult) > get<2>(bestResult)) 
			{
				if (level > 1) {
					uManager.MarkUnitForRemove(get<1>(bestResult));
					uManager.RemoveMarkedUnits();
				}
				cerr << "Creating attacker at " << get<1>(bestResult) << endl;
				me.CreateUnit(level, 1);
				m_answer.emplace_back(commands::Train(level, get<1>(bestResult)));
				// UPDATE UNITS
				uManager.AddUnit(0, -1, level, get<1>(bestResult)); // -1 is undef id
				map.m_map[get<1>(bestResult).y][get<1>(bestResult).x] = Tile::mActive;
				hasTerritory = true;
			}
		}
	}

private:
	Data* m_data;
	vector<pair<Vec2, Vec2> > m_mBridges, m_eBridges;
	// deduced:
	Vec2 m_mHQ, m_eHQ;

	set<Vec2>  m_takenPositions;
	vector<string> m_answer;

	CCSearch m_search;
	Bridges m_bridges;
};

struct Game {
	
	Game() : m_commander(&m_data) {};

	void Loop() {
		m_data.Init();
		while (true) {
			m_data.Update();
			m_commander.Clear();
			m_commander.Update();

			m_commander.Move();
			m_commander.Train();
			m_commander.Build();
			m_commander.Print();
		}
	}
private:
	Data m_data;
	Commander m_commander;
};

int main() {
	Game().Loop();
}