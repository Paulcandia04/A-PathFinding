#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

enum class AppState { MENU, PLAYING };

class GameState {
private:
    AppState m_state;
    bool m_editMode; // modo edición de calles caminables

public:
    GameState() : m_state(AppState::MENU), m_editMode(false) {}

    AppState GetState() const { return m_state; }
    void SetState(AppState s) { m_state = s; }

    bool IsEditMode() const { return m_editMode; }
    void ToggleEditMode() { m_editMode = !m_editMode; }
};

#endif