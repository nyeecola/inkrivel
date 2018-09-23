require "application_system_test_case"

class GameAccountsTest < ApplicationSystemTestCase
  setup do
    @game_account = game_accounts(:one)
  end

  test "visiting the index" do
    visit game_accounts_url
    assert_selector "h1", text: "Game Accounts"
  end

  test "creating a Game account" do
    visit game_accounts_url
    click_on "New Game Account"

    fill_in "Account", with: @game_account.account_id
    fill_in "Character", with: @game_account.character_id
    fill_in "Matches Played", with: @game_account.matches_played
    fill_in "Winning Matches", with: @game_account.winning_matches
    click_on "Create Game account"

    assert_text "Game account was successfully created"
    click_on "Back"
  end

  test "updating a Game account" do
    visit game_accounts_url
    click_on "Edit", match: :first

    fill_in "Account", with: @game_account.account_id
    fill_in "Character", with: @game_account.character_id
    fill_in "Matches Played", with: @game_account.matches_played
    fill_in "Winning Matches", with: @game_account.winning_matches
    click_on "Update Game account"

    assert_text "Game account was successfully updated"
    click_on "Back"
  end

  test "destroying a Game account" do
    visit game_accounts_url
    page.accept_confirm do
      click_on "Destroy", match: :first
    end

    assert_text "Game account was successfully destroyed"
  end
end
