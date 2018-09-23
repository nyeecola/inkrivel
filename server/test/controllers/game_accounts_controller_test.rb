require 'test_helper'

class GameAccountsControllerTest < ActionDispatch::IntegrationTest
  setup do
    @game_account = game_accounts(:one)
  end

  test "should get index" do
    get game_accounts_url
    assert_response :success
  end

  test "should get new" do
    get new_game_account_url
    assert_response :success
  end

  test "should create game_account" do
    assert_difference('GameAccount.count') do
      post game_accounts_url, params: { game_account: { account_id: @game_account.account_id, character_id: @game_account.character_id, matches_played: @game_account.matches_played, winning_matches: @game_account.winning_matches } }
    end

    assert_redirected_to game_account_url(GameAccount.last)
  end

  test "should show game_account" do
    get game_account_url(@game_account)
    assert_response :success
  end

  test "should get edit" do
    get edit_game_account_url(@game_account)
    assert_response :success
  end

  test "should update game_account" do
    patch game_account_url(@game_account), params: { game_account: { account_id: @game_account.account_id, character_id: @game_account.character_id, matches_played: @game_account.matches_played, winning_matches: @game_account.winning_matches } }
    assert_redirected_to game_account_url(@game_account)
  end

  test "should destroy game_account" do
    assert_difference('GameAccount.count', -1) do
      delete game_account_url(@game_account)
    end

    assert_redirected_to game_accounts_url
  end
end
