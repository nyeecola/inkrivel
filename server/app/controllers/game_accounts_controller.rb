class GameAccountsController < ApplicationController
  before_action :set_game_account, only: [:show, :edit, :update, :destroy]

  # GET /game_accounts
  # GET /game_accounts.json
  def index
    @game_accounts = GameAccount.all
  end

  # GET /game_accounts/1
  # GET /game_accounts/1.json
  def show
  end

  # GET /game_accounts/new
  def new
    @game_account = GameAccount.new
  end

  # GET /game_accounts/1/edit
  def edit
  end

  # POST /game_accounts
  # POST /game_accounts.json
  def create
    @game_account = GameAccount.new(game_account_params)

    respond_to do |format|
      if @game_account.save
        format.html { redirect_to @game_account, notice: 'Game account was successfully created.' }
        format.json { render :show, status: :created, location: @game_account }
      else
        format.html { render :new }
        format.json { render json: @game_account.errors, status: :unprocessable_entity }
      end
    end
  end

  # PATCH/PUT /game_accounts/1
  # PATCH/PUT /game_accounts/1.json
  def update
    respond_to do |format|
      if @game_account.update(game_account_params)
        format.html { redirect_to @game_account, notice: 'Game account was successfully updated.' }
        format.json { render :show, status: :ok, location: @game_account }
      else
        format.html { render :edit }
        format.json { render json: @game_account.errors, status: :unprocessable_entity }
      end
    end
  end

  # DELETE /game_accounts/1
  # DELETE /game_accounts/1.json
  def destroy
    @game_account.destroy
    respond_to do |format|
      format.html { redirect_to game_accounts_url, notice: 'Game account was successfully destroyed.' }
      format.json { head :no_content }
    end
  end

  private
    # Use callbacks to share common setup or constraints between actions.
    def set_game_account
      @game_account = GameAccount.find(params[:id])
    end

    # Never trust parameters from the scary internet, only allow the white list through.
    def game_account_params
      params.require(:game_account).permit(:account_id, :character_id, :matches_played, :winning_matches)
    end
end
