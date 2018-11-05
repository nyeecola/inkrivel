# This file is auto-generated from the current state of the database. Instead
# of editing this file, please use the migrations feature of Active Record to
# incrementally modify your database, and then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your
# database schema. If you need to create the application database on another
# system, you should be using db:schema:load, not running all the migrations
# from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended that you check this file into your version control system.

ActiveRecord::Schema.define(version: 2018_09_25_002253) do

  create_table "accounts", force: :cascade do |t|
    t.string "user"
    t.string "email"
    t.string "nickname"
    t.string "password_digest"
    t.date "last_login"
    t.boolean "connected"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

  create_table "characters", force: :cascade do |t|
    t.string "name"
    t.integer "hit_points"
    t.integer "damage"
    t.float "attack_speed"
    t.float "speed"
    t.string "model_file"
    t.string "texture_file"
    t.integer "hitbox_radius"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

  create_table "game_accounts", force: :cascade do |t|
    t.integer "account_id"
    t.integer "character_id", default: 0
    t.integer "matches_played", default: 0
    t.integer "winning_matches", default: 0
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["account_id"], name: "index_game_accounts_on_account_id"
  end

  create_table "game_players", force: :cascade do |t|
    t.integer "game_id"
    t.integer "game_account_id"
    t.string "state"
    t.integer "player_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["game_account_id"], name: "index_game_players_on_game_account_id"
    t.index ["game_id"], name: "index_game_players_on_game_id"
  end

  create_table "games", force: :cascade do |t|
    t.integer "map_id"
    t.integer "room_size"
    t.string "state"
    t.string "string"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["map_id"], name: "index_games_on_map_id"
  end

  create_table "maps", force: :cascade do |t|
    t.string "name"
    t.string "model_file"
    t.string "texture_file"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

end
