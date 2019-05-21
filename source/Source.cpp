#include <iostream>
#include <string>
#include <vector>
#include <array>
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
};

struct Player {

	void Read() {
		cin >> m_gold; cin.ignore();
		cin >> m_income; cin.ignore();
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
		return u.m_level < m_level;
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
		ss << "BUILD  " << toInt(ty) << " " << pos.x << " " << pos.y << ";";
		return ss.str();
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

		m_me.m_upkeep = 0;
		for (auto& unit : m_uManger.m_units) {
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
	}

	vector<pair<Vec2, Vec2>>&& GetBridges(Tile type) noexcept {
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
					// cerr << "Bridge: " << v << " - " << to << endl;
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
};


/* find components connected by bridge in the graph */
class CCSearch {
public:
	CCSearch(Map*map, UnitManager* manager) :
		m_map(map),
		m_uManager(manager),
		m_shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} }
	{//default ctor
	}

	int GetScoreAfterBridge(Vec2 start, Vec2 bridge, Tile type) noexcept {
		int score{ 0 };
		// CC before bridge
		this->Dfs(start, bridge, score, type);
		// CC after bridge with calc score eval
		score = 0;
		this->Dfs(bridge, Vec2{-1,-1}, score, type);
		return score;
	}
	/* used to calculate size of CC */
	template <class Pred>
	void Dfs(Vec2 v, int& visits, Pred pred) {
		static_assert(is_invocable_v<Pred>, "can't invoce predicate");
		m_visited[v.y][v.x] = true;
		visits++;
		for (auto& sh : m_shift) {
			auto to{ v + sh };
			if (!IsValid(to) || m_visited[to.y][to.x]) continue;
			auto ty{ m_map->Get(to) };
			if (pred(to))
				Dfs(to, visits, pred);
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

	void Dfs(Vec2 v, Vec2 bridge, int& score, Tile type) {
		m_visited[v.y][v.x] = true;

		auto cType{ m_map->Get(v) };
		auto optUnit{ m_uManager->GetUnitAt(v) };
// calc score:
		auto tileScore{ 0 };
		if (optUnit.has_value()) {
			// tile is with opponent's unit (weaker or equel in level)
			// [unit's cost + default enemy tile score]
			auto& unit{ optUnit.value() };
			tileScore = sd::costByLevel[unit.m_level - 1] + sd::activeTileScore;
		}
		else if (cType == Tile::eActive || cType == Tile::mActive) {
			tileScore = sd::activeTileScore;
		}
		else if (cType == Tile::eInactive || cType == Tile::mInactive) {
			tileScore = sd::inactiveTileScore;
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
private:
	array<Vec2, 4> m_shift;
	bool m_visited[Map::SIZE][Map::SIZE];
	Map* m_map;
	UnitManager* m_uManager;
};

class Commander {
public:	
	Commander(Data *data) :
		m_data(data)
	{
	}

	void Update() noexcept {
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

		Bridges bridge(&m_data->m_map);
		m_eBridges = bridge.GetBridges(Tile::eActive);
		m_mBridges = bridge.GetBridges(Tile::mActive);
	}

	void Clear() noexcept {
		m_takenPositions.clear();
		m_answer.clear();
		m_mBridges.clear();
		m_eBridges.clear();
	}
	/// TODO: when we need to level up or run
	/* Score:
	- enemy active tile: 
		- enemy active tile with HQ => MAX
		- enemy active tile with unit { level lesser or equel our }  =>  [ unit cost + activeTile * 2 ] 
		- enemy active tile with unit { level greater our }  =>  [ - MAX ]
		- enemy active tile => [ activeTile * 2 ]
	- enemy inactive tile
		- enemy inactive tile => [ inactiveTile(for enemy) + activeTile (for me) ]
	- neutral => activeTile
	- mInactive => activeTile
	- mActive => 0
	- tile with enemy unit { level greater or equel our } as neighbor [ -MAX ]
	*/
	int ScoreMove(Vec2 pos, int myLevel) noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };
		auto& map{ m_data->m_map };
		auto& uManager{ m_data->m_uManger };
		auto optUnit{ uManager.GetUnitAt(pos) };

		const int mx{ 1000 };
		const int mn{ -mx };

		Tile tileTy{ map.Get(pos) };
		int score{ 0 };

		if (pos == m_mHQ) return mn;
		if (pos == m_eHQ) return mx;
		if (tileTy == Tile::eActive) 
		{ // - enemy active tile with HQ => MAX
			 if (optUnit.has_value()) {
				int level{ optUnit.value().m_level };
				if (level <= myLevel)  // - enemy active tile with unit{ level lesser or equel our } = > [unit cost + activeTile * 2]
					score = sd::costByLevel[level - 1] + sd::activeTileScore * 2;
				else {// - enemy active tile with unit{ level greater our } = > [-MAX]
					cerr << pos << " is enemy with level >= our's" << endl;
					return mn;
				}
			}
			// - enemy active tile = > [activeTile * 2]
			score = max(score, sd::activeTileScore * 2);
		}
		else if (tileTy == Tile::eInactive) 
		{ // - enemy inactive tile => [ inactiveTile(for enemy) + activeTile (for me) ]
			score = sd::inactiveTileScore + sd::activeTileScore;
		}
		else if (tileTy == Tile::mInactive || tileTy == Tile::neutral) {
			if (optUnit.has_value()) score = 0; // if unit is trying to not move
			else score = sd::activeTileScore;
		}
		else if (tileTy == Tile::mActive) {
			score = 0;
		}
		// tile with enemy unit { level greater or equel our } as neighbor [ -MAX ]
		for (auto& sh : shift) {
			auto neighbor{ pos + sh };
			if (!IsValid(neighbor) || map.Get(neighbor) != Tile::eActive) continue;
			auto u{ uManager.GetUnitAt(neighbor) };
			if (u.has_value()) { //there is enemy unit
				int level { u.value().m_level };
				if (level >= myLevel) {
					cerr << pos << " has neighbor enemy with level >= our's" << endl;
					return mn; // can't make move there! enemy can strike! need to run!
					///TODO: need to increase priority for running!
				}
			}
		}
		return score;
	}

	/* MOVE:
	1. foreach my unit 
	2. give score to my neighbors and tile on already occupied tile (maybe it's better to not move):
	3. choose with max score
	4. add to answer array
	*/
	void Move() noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };

		auto& map{ m_data->m_map };
		auto& uManager{ m_data->m_uManger };
		auto& units{ uManager.m_units };

		std::random_device rd;
		std::mt19937 g(rd());
		shuffle(units.begin(), units.end(),g);

		for (auto& unit : units) {
			if (unit.IsMy()) {
				vector<pair<Vec2, int>> targets;
				targets.reserve(4); // max possible number of neighbors
				for (auto& sh : shift) {
					auto neighbor{ unit.m_pos + sh };
					
					if (!IsValid(neighbor)) continue; // in map

					auto u{ uManager.GetUnitAt(neighbor) };
					bool hasAllyUnit{ u.has_value() && u.value().IsMy() };// is occupied  by my unit

					if ( map.Get(neighbor) != Tile::blocked && //not blocked
						!hasAllyUnit && // is without my units
						m_takenPositions.count(neighbor) == 0 ) // isn't occupied  
					{ // calculate score:
						targets.emplace_back(neighbor, this->ScoreMove(neighbor, unit.m_level));
					}
				}
				//calculate score if we won't move
				auto stay{ make_pair(unit.m_pos, this->ScoreMove(unit.m_pos, unit.m_level)) };

				targets.emplace_back(stay);
				
				for_each(targets.begin(), targets.end(), [](auto&p) {
					cerr << p.first << " " << p.second << endl;
				});
				
				auto bestTarget = max_element(targets.begin(), targets.end(), [this](auto& l, auto& r) {
					if (l.second == r.second) {
						return l.first.Distanse(m_eHQ) > r.first.Distanse(m_eHQ);
					}
					return l.second < r.second;
				});
				m_answer.emplace_back(commands::Move(unit.m_id, bestTarget->first));
				m_takenPositions.insert(bestTarget->first);
				unit.m_pos = bestTarget->first; // update position
			}
		}
	}

	///TODO: add training for defence to interupt path of enemy
	/* TRAINING:
	Find all my boarder tiles 
	Foreach my tile check boarder tiles and give score:
		+ tile is enemy HQ [max score]
		+ tile is with opponent's unit (weaker or equel in level) [unit's cost + default enemy tile score]
		+ tile is enemy Bridge [number of opponent tiles + units cost if there are any in connected component]
		- tile is next to my Bridge [can reinforce bridge? YES = |CC|  + units cost there, NO = default tile score + 1] / 2
		+ tile is next to my inactive [|CC|] // build bridge 
		+ tile is opponents active [3] // default enemy tile score 
		+ tile is opponents inactive [2]
		+ else [1]
	Sort by score (if is equel, use distance to enemy HQ)
	While can train => train
	*/
	void Train() noexcept {
		auto tiles{ this->GetBoarderTiles() };
		// remove positions if there is already unit to move there.
		tiles.erase(
			remove_if(tiles.begin(), tiles.end(), [this](auto& p) {
				return (m_takenPositions.find(p.first) != m_takenPositions.end());
			}),
			tiles.end()
		);
		this->AssignScore(tiles);
		// While can train = > train
//		for (auto&[p, s] : tiles) {
//			cerr << "[ " << p << " " << s << " ], ";
//		}
//		cerr << endl;
		//---
		auto& me{ m_data->m_me };
		auto& map{ m_data->m_map };

		CCSearch cc(&map, &m_data->m_uManger);
//		cerr << "me0: " << me.m_gold << " " << me.m_income << " " << me.m_upkeep << endl;
		for (auto&tile : tiles)
		{ // TODO: change to DFS for mInactive!
			int income{ map.Get(tile.first) == Tile::mActive ? 0 : 1 };
			if (income) {
				cc.Dfs(tile.first, income, Tile::mInactive);
				cc.Clear();
			}
			if (me.CanCreateUnit(1, income)) {
				me.CreateUnit(1, income);
				m_takenPositions.insert(tile.first);
				m_answer.emplace_back(commands::Train(1, tile.first));
//				cerr << tile.first << " me: " << me.m_gold << " " << me.m_income << " " << me.m_upkeep << endl;
			}
			else break;
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
				if (IsValid(tile) && 
					!visited[tile.y][tile.x] && 
					map.Get(tile) != Tile::blocked )
				{
					visited[tile.y][tile.x] = true;

					if (map.Get(tile) == Tile::mActive) {
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

	void AssignScore(vector<pair<Vec2, int>> & boarderTiles) noexcept {
		for (auto & [pos,score] : boarderTiles) {
			score = this->EvalScore(pos);
		}
		sort(boarderTiles.begin(), boarderTiles.end(), [this](auto&lsh, auto&rsh) {
			if (lsh.second == rsh.second) {
				return lsh.first.Distanse(this->m_eHQ) < rsh.first.Distanse(this->m_eHQ);
			}
			else return lsh.second > rsh.second;
		});
	}
	
	int EvalScore(Vec2& pos) noexcept {
		array<Vec2, 4> shift{ Vec2{-1, 0}, {1, 0}, {0, -1}, {0, 1} };

		auto& map{ m_data->m_map };
		auto& bManager{ m_data->m_bManager };
		auto& uManager{ m_data->m_uManger };

		auto score{ sd::defaultScore };

		auto optUnit{ m_data->m_uManger.GetUnitAt(pos) };
		
		bool isEnemyBridge{
			find_if(m_eBridges.begin(), m_eBridges.end(), [&pos](auto&b) {
				return pos == b.second;
			}) != m_eBridges.end()
		};

		if (m_eHQ == pos) {
			return sd::mxScore;
		}
		else if (optUnit.has_value()) {
			// tile is with opponent's unit (weaker or equel in level)
			// [unit's cost + default enemy tile score]
			auto& unit{ optUnit.value() };
			score = sd::costByLevel[unit.m_level - 1] + sd::activeTileScore;
		}
		else if (map.Get(pos) == Tile::eActive) {
			score = sd::activeTileScore;
		}
		else if (map.Get(pos) == Tile::eInactive) {
			score = sd::inactiveTileScore;
		}
		// bridges:
		CCSearch cc(&map, &uManager);
		if (isEnemyBridge) {
			// tile is enemy Bridge. Score: 
			// [number of opponent tiles]
			// [units cost if there are any in connected component]
			score = max(score, cc.GetScoreAfterBridge(m_eHQ, pos, Tile::eActive));
			cc.Clear();
		}
		// TODO: 
		// check all neigbors for my bridge 
		// if we has bridge => reinforce!
		
		//for (auto&sh : shift) {
		//	auto neighbor{ sh + pos };
		//	if (!IsValid(neighbor)) continue;
		//	auto it = find_if(allyBridges.begin(), allyBridges.end(), [&neighbor,&pos](auto&b) {
		//		return neighbor == b.second;// neighbor is bridge
		//	});
		//	if (it != allyBridges.end() ) {
		////&????		// make sure we didn't come from this bridge(@neighbor) to @pos
		//	
		//		score = max(score, ccsearch.GetScoreAfterBridge(m_mHQ, neighbor, Tile::mActive) / 2);
		//	}
		//}

		// look for inactive component
		// how much we will get if activate?
		for (auto&sh : shift) {
			auto neighbor{ sh + pos };
			if (!IsValid(neighbor) || map.Get(neighbor) != Tile::mInactive) continue;
			int inactiveCount{ 0 };
			cc.Dfs(neighbor, inactiveCount, Tile::mInactive);
			cc.Clear();
			score = max(score, inactiveCount);
		}
		return score;
	}

private:
	Data* m_data;
	vector<pair<Vec2, Vec2> > m_mBridges, m_eBridges;
	// deduced:
	Vec2 m_mHQ, m_eHQ;

	set<Vec2>  m_takenPositions;
	vector<string> m_answer;
};

struct Game {
	
	Game() : m_commander(&m_data) {};

	void Loop() {
		m_data.Init();
		while (true) {
			m_data.Update();
			m_commander.Update();
			m_commander.Clear();

			m_commander.Move();
			m_commander.Train();
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