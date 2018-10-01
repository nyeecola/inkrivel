class GamesController < ApplicationController
  before_action :set_game, only: [:show, :edit, :update, :destroy]

  # GET /games
  # GET /games.json
  def index
    @games = Game.all
  end

  # GET /games/1
  # GET /games/1.json
  def show
  end

  # GET /games/new
  def new
    @game = Game.new
  end

  # GET /games/1/edit
  def edit
  end

  # POST /games
  # POST /games.json
  def create
    @game = Game.new(game_params)

    respond_to do |format|
      if @game.save
        format.html { redirect_to @game, notice: 'Game was successfully created.' }
        format.json { render :show, status: :created, location: @game }
      else
        format.html { render :new }
        format.json { render json: @game.errors, status: :unprocessable_entity }
      end
    end
  end

  # PATCH/PUT /games/1
  # PATCH/PUT /games/1.json
  def update
    respond_to do |format|
      if @game.update(game_params)
        format.html { redirect_to @game, notice: 'Game was successfully updated.' }
        format.json { render :show, status: :ok, location: @game }
      else
        format.html { render :edit }
        format.json { render json: @game.errors, status: :unprocessable_entity }
      end
    end
  end

  # DELETE /games/1
  # DELETE /games/1.json
  def destroy
    @game.destroy
    respond_to do |format|
      format.html { redirect_to games_url, notice: 'Game was successfully destroyed.' }
      format.json { head :no_content }
    end
  end

  # /games/join.json
  def join
    room_size = params[:room_size]
    account = Account.find(params[:account_id])
    games_list = Game.accepting_players(room_size)

    if games_list.length > 0
      game = games_list.first
    else
      game = Game.create(map: Map.random, room_size: room_size, state: 'waiting')
    end

    game.game_players.create(game_account: account.game_account, state: 'waiting')
    respond_to do |format|
      format.json { render json: { game_id: game.id } }
    end
  end

  # /games/leave.json
  def leave
    player = Account.find(params[:account_id]).active_game_player

    if player.present?
      player.destroy
    end

    respond_to do |format|
      format.json { head :ok }
    end
  end

  # /games/state
  def state
    player = Account.find(params[:account_id]).active_game_player
    game = player.game

    if game.ready_to_start?
      game.update(state: 'starting')
      player.update(state: 'starting')
    elsif game.game_ready?
      game.start
    elsif game.state == 'starting'
      player.update(state: 'starting')
    end

    respond_to do |format|
      format.json { render json: {
          state: game.state,
          waiting_players: game.required_players
      } }
    end
  end


  private
    # Use callbacks to share common setup or constraints between actions.
    def set_game
      @game = Game.find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def game_params
      params.require(:game).permit(:map_id, :room_size, :integer, :state, :string)
    end
end
